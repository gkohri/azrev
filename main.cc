
#include <cstdio>
#include <string>

#include "reviews.h"
#include "graph.h"

using std::string;

int main( int argc, char* argv[] ) {

    if ( argc < 2 ) {
        fprintf( stderr, "Usage: %s <metadata filename>\n", argv[0] );
        exit(1);
    }

    string output_dir = "Data/";
    string reviewer_index_filename = output_dir + "index_reviewers.csv";
    string edges_dir = output_dir + "tmp3";
    string edges_file = output_dir + "ar_edges.csv";
    string degree_dist_file = output_dir + "ar_degree_dist.csv";
    string evc_file = output_dir + "ar_evc.csv";
    string tmp_buckets = output_dir + "tmp4";
    string mat_file = output_dir + "ar_mat.bin";
    string cluster_mem_file = output_dir + "ar_cluster_mem.csv";

/*
    fprintf( stderr, "Loading the reviews...\n" );

    Reviews reviews( argv[1] );

    fprintf( stderr, "Number of reviewers: %zd\n", reviews.num_reviewers() );

    fprintf( stderr, "Condensing...\n" );

    long num_droped = reviews.condense_links();

    fprintf( stderr, "Number Condensed: %ld\n",num_droped);

    fprintf( stderr, "Reviewer stats...\n" );

    reviews.reviews_per_reviewer();

    fprintf( stderr, "Outputing reviewer index...\n" );

    reviews.output_reviewer_index( reviewer_index_filename );

    fprintf( stderr, "Mapping edges ...\n" );

    reviews.map_edges( edges_dir );

    fprintf( stderr, "Number of reviews: %zd\n",reviews.num_reviews() );
    fprintf( stderr, "Number of reviewers: %zd\n",reviews.num_reviewers() );
    fprintf( stderr, "Number of products: %zd\n",reviews.num_products() );
    fprintf( stderr, "Number of titles: %zd\n",reviews.num_titles() );

    fprintf( stderr, "Reducing edges ...\n" );
    Reviews::reduce_edges( edges_dir, edges_file );

    fprintf( stderr, "Degree distribution ...\n" );

    Graph graph( reviews.num_reviewers() );
    graph.degree_dist( edges_file, degree_dist_file );
    graph.eigen_vect_cent( edges_file, evc_file, 20, 1.0e-10 );

    graph.convert_list_to_mat( edges_file,
                               degree_dist_file,
                               evc_file,
                               tmp_buckets,
                               mat_file );
*/

    Graph graph( 2588990 );
    graph.cluster_stats( edges_file, cluster_mem_file );
}
