#include "Feed.h"
#include "debug.h"
#include "utils.h"
#include "exceptions.h"

std::ostream &operator<<(std::ostream &os, const Feed &feed) {
    os << "*** " << feed.get_title() << " ***" << std::endl;

    auto entries = feed.get_entries();
    for (const auto &entry : entries) {
        os << entry.title << std::endl;
        os << "URL: " << entry.url << std::endl;
        os << "Aktualizace: " << entry.time << std::endl;
        os << "Autor: " << entry.author << std::endl << std::endl;
    }
    return os;
}

Feed::Feed(const string &source_url) : source(source_url) {
    this->is_atom = false;
}

void Feed::parse(const string &xml) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(xml.c_str(), xml.length());
    if (!result) {
        throw ApplicationException(this->source+": Zdroj nevrátil validní XML (chyba: "+string(result.description())+")");
    }

    pugi::xml_node feed_node = doc.first_child();
    this->parse_namespaces(feed_node);

    if (!this->is_valid_feed_node(feed_node)) {
        throw ApplicationException(this->source+": Formát zdroje není validní (kořenový XML element není validní)");
    }

    if (feed_node.name() == this->atom("feed"))
        this->is_atom = true;

    this->parse_feed_title(feed_node);
    this->parse_entries(feed_node);
}

bool Feed::is_valid_feed_node(pugi::xml_node &root_node) const {
    string name = root_node.name();
    return name == this->atom("feed") ||
           name == "rss" ||
           name == this->rdf("RDF") ||
           name == "rdf:RDF";
}

void Feed::parse_feed_title(const pugi::xml_node &feed_node) {
    pugi::xml_node title_node;
    if ((title_node = feed_node.child(this->atom("title").c_str()))) {}
    else if ((title_node = feed_node.child("channel").child("title"))) {}
    else {
        this->title = "<< BEZ NÁZVU >>";
        return;
    }

    this->title = title_node.text().as_string("<< CHYBA ČTENÍ NÁZVU >>");
}

const string &Feed::get_title() const {
    return this->title;
}

const vector<FeedEntry> Feed::get_entries() const {
    return this->entries;
}

void Feed::parse_entries(const pugi::xml_node &feed_node) {
    this->prefixed_atom_entry = this->atom("entry");

    auto it = this->get_entry_iterator(feed_node);
    if (it.begin() == it.end())
        return;

    FeedEntry feed_entry;
    for (const auto &entry : it) {
        feed_entry = FeedEntry();
        feed_entry.title = this->get_entry_title(entry);
        feed_entry.time = this->get_entry_time(entry);
        feed_entry.url = this->get_entry_url(entry);
        feed_entry.author = this->get_entry_author(entry);

        this->entries.push_back(feed_entry);
    }
}

pugi::xml_object_range<pugi::xml_named_node_iterator> Feed::get_entry_iterator(const pugi::xml_node &feed_node) const {
    auto it = feed_node.children("item");
    if (it.begin() != it.end()) return it;
    it = feed_node.children(this->prefixed_atom_entry.c_str());
    if (it.begin() != it.end()) return it;
    it = feed_node.child("channel").children("item");
    return it;
}

string Feed::get_entry_title(const pugi::xml_node &entry_node) const {
    if (entry_node.child(this->atom("title").c_str()))
        return entry_node.child(this->atom("title").c_str()).text().as_string("<< BEZ NÁZVU >>");
    return entry_node.child("title").text().as_string("<< BEZ NÁZVU >>");
}

string Feed::get_entry_time(const pugi::xml_node &entry_node) const {
    pugi::xml_node time_node;
    if ((time_node = entry_node.child(this->dc("date").c_str()))) {}
    else if ((time_node = entry_node.child("dc:date"))) {}
    else if ((time_node = entry_node.child("pubDate"))) {}
    else if ((time_node = entry_node.child(this->atom("updated").c_str()))) {}
    return time_node.text().as_string();
}

string Feed::get_entry_url(const pugi::xml_node &entry_node) const {
    pugi::xml_node url_node = entry_node.child("guid");

    if ((url_node && !url_node.attribute("isPermaLink")) ||
            (url_node.attribute("isPermaLink")
             && url_node.attribute("isPermaLink").as_bool(true)))
    {
        return url_node.text().as_string();
    }

    // If feed is in atom format or has link from atom namespace, find alternate link and return href
    const string &atom_link = this->atom("link");
    if (this->is_atom || (!this->ns_atom.empty() && entry_node.child(atom_link.c_str()))) {
        for (const auto link_node : entry_node.children(atom_link.c_str())) {
            if (string(link_node.attribute("rel").as_string("alternate")) == "alternate") {
                return link_node.attribute("href").as_string();
            }
        }
    }

    url_node = entry_node.child("link");
    return url_node.text().as_string();
}

string Feed::get_entry_author(const pugi::xml_node &entry_node) const {
    pugi::xml_node author_node;

    const string &atom_author = this->atom("author");
    if (this->is_atom || (!this->ns_atom.empty() && entry_node.child(atom_author.c_str()))) {
        auto it = entry_node.children(atom_author.c_str());
        if (it.begin() != it.end()) {
            return this->get_atom_entry_authors(it);
        }

        it = entry_node.child(this->atom("source").c_str()).children(atom_author.c_str());
        if (it.begin() != it.end()) {
            return this->get_atom_entry_authors(it);
        }

        it = entry_node.parent().children(atom_author.c_str());
        if (it.begin() != it.end()) {
            return this->get_atom_entry_authors(it);
        }
    }

    if ((author_node = entry_node.child(this->dc("creator").c_str()))) {}
    else if ((author_node = entry_node.child("author"))) {}
    else if ((author_node = entry_node.child("dc:creator"))) {}

    return author_node.text().as_string();
}

string Feed::get_atom_entry_authors(const pugi::xml_object_range<pugi::xml_named_node_iterator> &it) const {
    string authors{};
    string splitter{};
    for (const auto atom_author_node : it) {
        if (atom_author_node.child("name").text()) {
            authors += splitter;
            authors += atom_author_node.child("name").text().as_string();
            splitter = "; ";
        }
    }

    return authors;
}

void Feed::parse_namespaces(const pugi::xml_node &root_node) {
    string attr_val;
    for (const auto &attr : root_node.attributes()) {
        if (str_starts_with(attr.name(), "xmlns:")) {
            attr_val = attr.as_string();
            if (attr_val == "http://purl.org/dc/elements/1.1/") {
                this->ns_dc = this->get_ns_prefix(attr.name());
            } else if (attr_val == "http://www.w3.org/1999/02/22-rdf-syntax-ns#") {
                this->ns_rdf = this->get_ns_prefix(attr.name());
            } else if (attr_val == "http://www.w3.org/2005/Atom") {
                this->ns_atom = this->get_ns_prefix(attr.name());
            }
        }
    }
}

string Feed::get_ns_prefix(const string &attr_name) const {
    auto offset = attr_name.find(':');
    if (offset == string::npos)
        return "";

    return attr_name.substr(offset+1);
}

string Feed::dc(const string &str) const {
    if (this->ns_dc.empty())
        return str;
    return this->ns_dc + ':' + str;
}

string Feed::rdf(const string &str) const {
    if (this->ns_rdf.empty())
        return str;
    return this->ns_rdf + ':' + str;
}

string Feed::atom(const string &str) const {
    if (this->ns_atom.empty())
        return str;
    return this->ns_atom + ':' + str;
}
