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
    void parse(const string &xml);
    bool has_error() const;
    string get_error() const;

    const string& get_title() const;
    const vector<FeedEntry> get_entries() const;

private:
    pugi::xml_node get_feed_node(const pugi::xml_document &doc) const;
    void parse_feed_title(const pugi::xml_node &feed_node);

    string title;
    vector<FeedEntry> entries;

    string error;
};

std::ostream& operator<<(std::ostream &os, const Feed &feed);


#endif //ISA_FEED_H
