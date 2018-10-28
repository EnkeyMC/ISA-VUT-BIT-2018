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

void Feed::parse(const string &xml) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(xml.c_str(), xml.length());
    if (!result) {
        throw ApplicationException("Zdroj není validní XML (chyba: "+string(result.description())+")");
    }

    pugi::xml_node feed_node = this->get_feed_node(doc);
    if (!feed_node) {
        throw ApplicationException("Formát zdroje není validní (chybí validní kořenový XML element)");
    }

    this->parse_feed_title(feed_node);
    this->parse_entries(feed_node);
}

pugi::xml_node Feed::get_feed_node(const pugi::xml_document &doc) const {
    pugi::xml_node feed_node;
    if ((feed_node = doc.child("feed"))) {}
    else if ((feed_node = doc.child("rss"))) {}
    else if ((feed_node = doc.child("RDF"))) {}
    else if ((feed_node = doc.child("rdf:RDF"))) {}
    return feed_node;
}

void Feed::parse_feed_title(const pugi::xml_node &feed_node) {
    pugi::xml_node title_node;
    if ((title_node = feed_node.child("title"))) {}
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
    it = feed_node.children("entry");
    if (it.begin() != it.end()) return it;
    it = feed_node.child("channel").children("item");
    return it;
}

string Feed::get_entry_title(const pugi::xml_node &entry_node) const {
    return entry_node.child("title").text().as_string("<< BEZ NÁZVU >>");
}

string Feed::get_entry_time(const pugi::xml_node &entry_node) const {
    pugi::xml_node time_node;
    if ((time_node = entry_node.child("dc:date"))) {}
    else if ((time_node = entry_node.child("pubDate"))) {}
    else if ((time_node = entry_node.child("updated"))) {}
    return time_node.text().as_string();
}

string Feed::get_entry_url(const pugi::xml_node &entry_node) const {
    pugi::xml_node url_node = entry_node.child("link");
    if (url_node.text())
        return url_node.text().as_string();
    return url_node.attribute("href").as_string();
}

string Feed::get_entry_author(const pugi::xml_node &entry_node) const {
    pugi::xml_node author_node;
    if ((author_node = entry_node.child("dc:creator"))) {}
    else if ((author_node = entry_node.child("author").child("name"))) {}  // TODO print name AND email?
    else if ((author_node = entry_node.child("author").child("email"))) {}
    else if ((author_node = entry_node.child("author"))) {}
    else if ((author_node = entry_node.child("creator"))) {}

    return author_node.text().as_string();
}
