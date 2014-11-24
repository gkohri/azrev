#ifndef REVIEWS_H
#define REVIEWS_H

#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct metadata {

    long product_id;
    long reviewer_id;
    float score;
    short help;
    short outof;
    long time;

};

class Reviews {

 private:
    std::vector<metadata> _reviews;

    std::vector<std::string> reviewers;
    std::vector<std::string> screen_names;
    std::vector<std::string> products;
    std::vector<std::string> titles;

    std::unordered_map<std::string, size_t> rev_index;
    std::unordered_map<std::string, size_t> prod_index;
    std::unordered_map<std::string, size_t> title_index;


    std::vector<long> rev_num_revs;
    std::unordered_map< size_t, std::unordered_set<size_t> > prod_rev;
    std::unordered_map< size_t, std::unordered_set<size_t> > title_prod;
    
    std::string _filename;

 public:
    Reviews( const std::string &filename );
    Reviews( const char *filename );

    size_t num_reviews() {
        size_t _num_revs = 0;
        for ( std::pair<size_t, std::unordered_set<size_t> > pr : prod_rev ) {
            _num_revs += pr.second.size();    
        }
        return _num_revs;
    }

    size_t num_reviewers() {
        return rev_index.size();
    }
    size_t num_products() {
        return prod_rev.size();
    }
    size_t num_titles() {
        return title_index.size();
    }

    long condense_links();
    void reviews_per_reviewer();
    void output_reviewer_index( const std::string &filename );
    void map_edges( const std::string &dirname );

    static void reduce_edges( const std::string &mapdir, 
                              const std::string &redfile );

 private:
    void load_reviews( const std::string &filename );


};

#endif // REVIEWS_H
