#ifndef ISA_FEED_H
#define ISA_FEED_H

#include <ostream>
#include <string>
#include <vector>
#include "pugixml.hpp"

using std::string;
using std::vector;

typedef struct stFeedEntry {
    string title;
    string time;
    string author;
    string url;
} FeedEntry;


class Feed {
public:
    /**
     * Feed constructor
     * @param source_url feed source
     */
    explicit Feed(const string &source_url = "");

    /**
     * Parse feed from xml
     * @param xml XML string
     */
    void parse(const string &xml);

    /**
     * Get feed title
     * @return title
     */
    const string& get_title() const;

    /**
     * Get feed entries
     * @return vector of feed entries
     */
    const vector<FeedEntry> get_entries() const;

private:
    /**
     * Check if given root node is valid feed node
     * @param root_node root node of the XML
     * @return true if is valid, false otherwise
     */
    bool is_valid_feed_node(pugi::xml_node &root_node) const;

    /**
     * Parse namespace prefixes from root node
     * @param root_node XML root node
     */
    void parse_namespaces(const pugi::xml_node &root_node);

    /**
     * Get namespace prefix from attribute name
     * @param attr_name attribute name (ex. xmlns:dc)
     * @return namespace prefix if there is one, empty string otherwise
     */
    string get_ns_prefix(const string &attr_name) const;

    /**
     * Parse feed title from feed XML node
     * @param feed_node feed XML node
     */
    void parse_feed_title(const pugi::xml_node &feed_node);

    /**
     * Parse feed entries
     * @param feed_node feed XML node
     */
    void parse_entries(const pugi::xml_node &feed_node);

    /**
     * Get feed entry XML node iterator
     * @param feed_node feed XML node
     * @return entry XML node iterator
     */
    pugi::xml_object_range<pugi::xml_named_node_iterator> get_entry_iterator(const pugi::xml_node &feed_node) const;

    /**
     * Get entry title from entry XML node
     * @param entry_node entry XML node
     * @return entry title
     */
    string get_entry_title(const pugi::xml_node &entry_node) const;

    /**
     * Get feed entry time from entry XML node
     * @param entry_node entry XML node
     * @return entry time
     */
    string get_entry_time(const pugi::xml_node &entry_node) const;

    /**
     * Get entry URL from entry XML node
     * @param entry_node entry XML node
     * @return entry URL
     */
    string get_entry_url(const pugi::xml_node &entry_node) const;

    /**
     * Get entry author from entry XML node
     * @param entry_node entry XML node
     * @return entry author
     */
    string get_entry_author(const pugi::xml_node &entry_node) const;

    /**
     * Parses author names from iterator of <author> XML nodes.
     * Skips authors without specified name.
     * @param it XML node iterator
     * @return string of names (ex. "John Doe, Jane Poe")
     */
    string get_atom_entry_authors(const pugi::xml_object_range<pugi::xml_named_node_iterator> &it) const;

    /**
     * Prefix string with Dublin Core XML namespace if defined
     * @param str string to prefix
     * @return prefixed string
     */
    string dc(const string &str) const;

    /**
     * Prefix string with RDF XML namespace if defined
     * @param str string to prefix
     * @return prefixed string
     */
    string rdf(const string &str) const;

    /**
     * Prefix string with Atom XML namespace if defined
     * @param str string to prefix
     * @return prefixed string
     */
    string atom(const string &str) const;

    /**
     * Feed title
     */
    string title;

    /**
     * Feed entries
     */
    vector<FeedEntry> entries;

    /**
     * RDF namespace prefix
     */
    string ns_rdf;

    /**
     * Dublin Core namespace prefix
     */
    string ns_dc;

    /**
     * Atom namespace prefix
     */
    string ns_atom;

    /**
     * Feed source
     */
    string source;

    /**
     * Whether feed is atom or not
     */
    bool is_atom;

    /**
     * Atom entry tag name prefixed with atom namespace.
     *
     * We need to keep it in memory to return iterator from method.
     */
    string prefixed_atom_entry;
};

std::ostream& operator<<(std::ostream &os, const Feed &feed);


#endif //ISA_FEED_H
