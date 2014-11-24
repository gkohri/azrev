
#ifndef MISC_H
#define MISC_H

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

/**
 * A simple function for tokenizing a string based upon a set of delimiters.
 * By default, any white-space character can be used a delimiter: " \t\f\v\n\r"
 */
template <class ContainerT>
inline void tokenize(const std::string& str, ContainerT& tokens,
                     const std::string& delimiters = " \t\f\v\n\r" ) {

    typedef ContainerT Base; 
    typedef typename Base::value_type ValueType; 
    typedef typename ValueType::size_type SizeType;

    SizeType pos, last_pos = 0;

    while (true) {

        pos = str.find_first_of( delimiters, last_pos );

        if (pos == std::string::npos) {
            pos = str.length();
            if ( pos != last_pos ) {
                tokens.push_back( ValueType( str.data()+last_pos, 
                                               (SizeType)pos-last_pos ) );
            }
            break;
        } else {
            if ( pos != last_pos ) {
                tokens.push_back( ValueType( str.data()+last_pos, 
                                                (SizeType)pos-last_pos ) );
            }
        }

        last_pos = pos + 1;
    }
}


/**
 * Trim white spaces surrounding strings
 */
inline std::string trim( const std::string &s ) {
    size_t last_non_white = s.find_last_not_of( " \t\v\f\n\r" );
    if ( last_non_white == std::string::npos ) return std::string();

    size_t first_non_white = s.find_first_not_of( " \t\v\f\n\r" );
    if ( first_non_white == std::string::npos ) first_non_white = 0;

    return s.substr( first_non_white, ( last_non_white - first_non_white + 1 ));
};

/**
 * A function for combing a new value into a previous hash
 */

template <class T>
inline void hash_combine( std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

/**
 * A function for hasing a pair of integers
 */

namespace std {

template <>
struct hash<std::pair<int, int>> {
public:
    inline size_t operator()( std::pair<int, int> x ) const throw() {
        std::hash<int> hash_int;
        size_t h = hash_int( x.first );
        hash_combine( h, x.second );
        return h;
    }
};

}


/**
 * A fast method for reading in positive integers.
 * Use with caution as few checks are made for input errors.
 */
template <typename T>
void scan_pos_int( FILE *fp, T &x ) {

    int c = getc_unlocked( fp );

    if ( c == EOF ) {
        x = EOF;
        return;
    }

    x = 0;

    for ( ; ( c < 48 || c > 57 ); c = getc_unlocked( fp ) );

    for ( ; c > 47 && c < 58; c = getc_unlocked( fp ) ) {
        x = ( x << 1 ) + ( x << 3 ) + (c - 48);
    }
}

template <typename T>
std::vector<size_t> sort_indexes( const T *v, const int len) {

  // initialize original index locations
    std::vector<size_t> idx( len );
    for (size_t i = 0; i != len; ++i) idx[i] = i;

  // sort indexes based on comparing values in v
    std::sort( idx.begin(), idx.end(),
                [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

    return idx;
}

template <typename T>
std::vector<size_t> sort_indexes( const std::vector<T> &v ) {

  // initialize original index locations
    std::vector<size_t> idx(v.size());
    for (size_t i = 0; i < v.size(); ++i) idx[i] = i;

  // sort indexes based on comparing values in v
    std::sort( idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

    return idx;
}

inline
FILE* fopen_csv( const std::string &filename, 
                 const std::string &mode, 
                 const bool discard_header = true ){

    FILE *fp = fopen( filename.c_str(), mode.c_str() );

    if ( fp == NULL ) {
        fprintf( stderr, "Could not open file: %s\n", filename.c_str() );
        abort();
    }

    if ( mode == "r" && discard_header ) {

        char *line = NULL;
        size_t len = 0;

        int read = getline( &line, &len, fp );

        if ( read == EOF ) {
            fprintf(stderr,"Bad CSV file\n");
            fclose( fp );
            abort();
        }

        free( line );

    }

    return( fp );
}

struct MapCompare : public std::binary_function<size_t,size_t,bool> {

    std::unordered_map<size_t,size_t> *map_;

    MapCompare( std::unordered_map<size_t,size_t> *map ) {
        map_ = map;
    }

    bool operator() ( const size_t a,  const size_t b) const {
        size_t a_r = std::numeric_limits<size_t>::min();
        if ( map_->count(a) ) a_r = map_->at( a );
        size_t b_r = std::numeric_limits<size_t>::min();
        if ( map_->count(b) ) b_r = map_->at( b );
        return ( a_r < b_r);
    }

    bool operator() ( const std::pair<size_t,size_t> a,
                                    const std::pair<size_t,size_t> b) const {
        size_t a_r = std::numeric_limits<size_t>::min();
        if ( map_->count(a.first) ) a_r = map_->at( a.first );
        size_t b_r = std::numeric_limits<size_t>::min();
        if ( map_->count(b.first) ) b_r = map_->at( b.first );
        return ( a_r < b_r);
    }
};



#endif   // MISC_H

