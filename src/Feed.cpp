#include "Feed.h"
#include "debug.h"
#include "utils.h"

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
        this->error = result.description();
        return;
    }
    debug(result.status);
    debug(result.description());

    pugi::xml_node feed_node = this->get_feed_node(doc);
    if (!feed_node) {
        this->error = "Invalid feed format (missing valid root XML tag)";
        return;
    }

    this->parse_feed_title(feed_node);
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
        this->title = "<< ERROR: TITLE NOT FOUND >>";
        return;
    }

    this->title = title_node.text().as_string("<< ERROR READING TITLE >>");
}

const string &Feed::get_title() const {
    return this->title;
}

const vector<FeedEntry> Feed::get_entries() const {
    return this->entries;
}
