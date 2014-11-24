#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "misc.h"
#include "reviews.h"

using std::next;
using std::pair;
using std::string;
using std::stoi;
using std::unordered_map;
using std::unordered_set;
using std::vector;


Reviews::Reviews( const std::string &filename ) {
    _filename = filename;

    load_reviews( _filename );
}

Reviews::Reviews( const char *filename ) {
    _filename = string( filename );

    load_reviews( _filename );
}

void Reviews::load_reviews( const string &filename ) {

    FILE *fp = fopen_csv( filename, "r" );

    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    long num_prod = 0;
    long num_rev = 0;
    long num_tit = 0;
    while ( ( read = getline( &line, &len, fp ) ) != -1 ) {

        char *begin = line;
        char *end = index( line, '\t' );
        string product_id = string( begin, (end-begin) );

        begin = end + 1;
        end = index( begin, '\t' );
        string title = string( begin, (end-begin) );

        long prod_id = num_prod;
        if ( prod_index.count( product_id ) ) {
            prod_id = prod_index[product_id];
        } else {
            prod_index[product_id] = prod_id;
            products.push_back( product_id ); 
            ++num_prod;
        }

        long tit_id = num_tit;
        if ( title_index.count( title ) ) {
            tit_id = title_index[ title ];
        } else {
            title_index[ title ] = tit_id;
            titles.push_back( title );
            ++num_tit;
        }

        title_prod[tit_id].insert( prod_id );

        begin = end + 1;
        end = index( begin, '\t' );
        string reviewer_id = string( begin, (end-begin) );

        begin = end + 1;
        end = index( begin, '\t' );
        string screen_name = string( begin, (end-begin) );

        long rev_id = num_rev;
        if ( rev_index.count( reviewer_id ) ) {
            rev_id = rev_index[reviewer_id];
        } else {
            rev_index[reviewer_id] = rev_id;
            reviewers.push_back( reviewer_id ); 
            screen_names.push_back( screen_name ); 
            ++num_rev;
        }

        prod_rev[prod_id].insert( rev_id );

        begin = end + 1;
        end = index( begin, '\t' );
        string helpfulness = string( begin, (end-begin) );
        size_t slash = helpfulness.find( '/' );
        int help = stoi( helpfulness.substr( 0, slash ) );
        int outof = stoi( helpfulness.substr( slash+1 ) );

        char *last = NULL;

        begin = end + 1;
        end = index( begin, '\t' );
        float score = strtof( begin, &last );

        begin = end + 1;
        end = index( begin, '\t' );
        long time = strtol( begin, &last, 10 );

        _reviews.emplace_back();

        _reviews.rbegin()->product_id = prod_id;
        _reviews.rbegin()->reviewer_id = rev_id;
        _reviews.rbegin()->score = score;
        _reviews.rbegin()->time = time;
        _reviews.rbegin()->help = help;
        _reviews.rbegin()->outof = outof;

    }

    free( line );

    fclose(fp);
}

long Reviews::condense_links() {

    long num_droped = 0;
    for ( pair<size_t,unordered_set<size_t>> prods : title_prod ) {

        if ( prods.second.size() > 1 ) {

            unordered_set<size_t>::iterator iusit, jusit;

            for( iusit = prods.second.begin(); iusit != prods.second.end(); 
                                                                ++iusit ) {
                for( jusit = next(iusit); jusit != prods.second.end();++jusit){
                    if ( prod_rev[*iusit] == prod_rev[*jusit] ) {
                        num_droped += prod_rev.erase( *jusit );
                    }
                }
            }

        }

    }

    return( num_droped );
}

void Reviews::reviews_per_reviewer() {

    rev_num_revs.clear();
    rev_num_revs.resize( reviewers.size() );

    for ( pair<size_t,unordered_set<size_t>> pr : prod_rev ) {
        for ( size_t rev : pr.second ) {
            rev_num_revs[rev] += 1; 
        }
    }

}

void Reviews::output_reviewer_index( const string &filename ) {

    FILE *fp = fopen_csv( filename, "w", false );

    fprintf(fp,"vertexID\treviewerID\tScreenName\treviews\n");
    for( size_t r = 0; r < reviewers.size(); ++r ) {
        fprintf(fp,"%zd\t%s\t%s\t%ld\n", r, reviewers[r].c_str(),
                                       screen_names[r].c_str(),
                                       rev_num_revs[r] );
    }

    fclose(fp);


}

void Reviews::map_edges( const string &dirname ) {

    string bucket_filenames[127];
    string base = dirname + "/bucket_";

    for ( int i = 0; i < 127; ++i ) {
        if ( i < 10 ) {
            bucket_filenames[i] = base + "00" + std::to_string( i );
        } else if ( i < 100 ) {
            bucket_filenames[i] = base + "0" + std::to_string( i );
        } else {
            bucket_filenames[i] = base + std::to_string( i );
        }
    }

    FILE *buckets[127];
    for ( int i = 0; i < 127; ++i ) {
        buckets[i] = fopen_csv( bucket_filenames[i], "w", false );
    }

    unordered_map<size_t,unordered_set<size_t>>::iterator umit;
    unordered_set<size_t>::iterator sit_i, sit_j;

    std::hash<size_t> hash_st;

    for( umit = prod_rev.begin(); umit != prod_rev.end(); ++umit ) {
        if ( umit->second.size() > 1 ) {
            for( sit_i = umit->second.begin(); sit_i != umit->second.end();
                                                                    ++sit_i ) {
                for( sit_j = next(sit_i); sit_j != umit->second.end();
                                                                    ++sit_j ) {
                    if ( *sit_i < *sit_j ) {
                        size_t rev_hash = hash_st( *sit_i );
                        int bucket =  static_cast<int>( rev_hash % 127ul );
                        fprintf(buckets[bucket],"%zd\t%zd\n", *sit_i, *sit_j );
                    } else {
                        size_t rev_hash = hash_st( *sit_j );
                        int bucket =  static_cast<int>( rev_hash % 127ul );
                        fprintf(buckets[bucket],"%zd\t%zd\n", *sit_j, *sit_i );
                    }
                }
            }
        }
    }


    for ( int i = 0; i < 127; ++i ) {
        fclose( buckets[i] );
    }
}

void Reviews::reduce_edges( const std::string &mapdir, 
                            const std::string &redfile ) {


    string bucket_filenames[127];
    string base = mapdir + "/bucket_";

    for ( int i = 0; i < 127; ++i ) {
        if ( i < 10 ) {
            bucket_filenames[i] = base + "00" + std::to_string( i );
        } else if ( i < 100 ) {
            bucket_filenames[i] = base + "0" + std::to_string( i );
        } else {
            bucket_filenames[i] = base + std::to_string( i );
        }
    }


    FILE *output = fopen_csv( redfile, "w", false );
    fprintf( output, "source\ttarget\tweight\n" );

    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    for ( int b = 0; b < 127; ++b ) {

        FILE *bucket = fopen_csv( bucket_filenames[b], "r", false );

        unordered_map<string,long> edges;
        
        while( ( read = getline( &line, &len, bucket ) ) != -1 ) {
            edges[string(line,(read-1))] += 1;
        }

        fclose( bucket );

        for ( pair<string,long> edge : edges ) {
            fprintf( output,"%s\t%ld\n",edge.first.c_str(),edge.second);
        }

    }

    free(line);
        
    fclose( output );
}
