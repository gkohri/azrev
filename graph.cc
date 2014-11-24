
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <functional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <utility>
#include <vector>

#include "misc.h"
#include "graph.h"

using std::map;
using std::pair;
using std::stack;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

/**
 * A simple rountine for creating the degree distribution in an
 * out-of-core network.
 */ 

void Graph::degree_dist( const string &edge_filename, 
                         const string &output_filename ) {

    vector<size_t> degree( _num_verts );

    FILE *edge_file = fopen_csv( edge_filename, "r" );

    size_t source;
    size_t target;
    size_t weight;

    while( 1 ) {

        scan_pos_int( edge_file, source );
        scan_pos_int( edge_file, target );
        scan_pos_int( edge_file, weight );

        if ( weight == EOF ) break;

        ++degree[source];
        ++degree[target];

    }

    fclose( edge_file );

    FILE *output = fopen_csv( output_filename, "w", false );

    fprintf( output, "node\tdegree\n" );
    for ( size_t v = 0; v < _num_verts; ++v ) {
        fprintf( output, "%zd\t%zd\n", v, degree[v] );
    }
    fclose( output );
}

void Graph::eigen_vect_cent( const string &edge_filename, 
                             const string &output_filename,
                             const int num_it,
                             const double eps ) {



    FILE *edge_file = fopen_csv( edge_filename, "r" );

    double *rold = new double[_num_verts];
    double *rnew = new double[_num_verts];
    double *tmp;

    double dnorm = sqrt( 1.0/static_cast<double>(_num_verts) );
    for ( size_t v = 0; v < _num_verts; ++v ){
        rold[v] = dnorm;
    }
    
    double wnorm = 1.0;
    long   wsq = 0;

    size_t source;
    size_t target;
    size_t weight;

    char *line = NULL;
    size_t len = 0;

    double norm_last = 1.0;
    double delta = 1.0;
    int it;

    for ( it = 0; it < num_it; ++it ) {

        fprintf(stderr,"%3d %14.7e %14.7e\n",it,delta,norm_last);

        rewind( edge_file );
    

        //pop the header line
        int read = getline( &line, &len, edge_file );

        if ( read == EOF ) {
            fprintf(stderr,"Bad file\n");
            fclose( edge_file );
            abort();
        }

        memset( rnew, 0, _num_verts*sizeof(double) );

        

        while( 1 ) {

            scan_pos_int( edge_file, source );
            scan_pos_int( edge_file, target );
            scan_pos_int( edge_file, weight );

            if ( weight == EOF ) break;

            double dweight = static_cast<double>(weight)*wnorm;

            rnew[source] += dweight*rold[target];
            rnew[target] += dweight*rold[source];

            if ( it == 0 ) {
                wsq += weight*weight;
            }

        }

        wnorm = 1.0/sqrt( static_cast<double>(wsq) );

        double norm_sq = 0.0;
        for ( size_t i = 0; i < _num_verts; ++i ){
            norm_sq += rnew[i]*rnew[i];
        }
        double norm = sqrt( norm_sq );
        double inv_norm = 1.0/norm;
        for ( size_t i = 0; i < _num_verts; ++i ){
            rnew[i] *= inv_norm;
        }

        delta = fabs( (norm - norm_last) )/norm_last;
        if ( delta < eps ) break;

        norm_last = norm;

        tmp = rold;
        rold = rnew;
        rnew = tmp;
    }

    fclose( edge_file );

    fprintf(stderr,"num iterations: %d\n", it );
    fprintf(stderr,"eigenvalue: %14.7e\n", norm_last );

    vector<size_t> ranks = sort_indexes( rnew, _num_verts );

    FILE *output = fopen_csv( output_filename, "w", false );

    fprintf( output, "node\trank\n" );
    for ( size_t v = 0; v < _num_verts; ++v ) {
        fprintf( output, "%zd\t%zd\n", ranks[v], (v+1) );
    }
    fclose( output );

    delete[] rnew;
    delete[] rold;
}

void Graph::cluster_stats( const string &edge_filename, 
                           const string &output_filename ) {

    FILE *edge_fp = fopen_csv( edge_filename, "r" );

    vector<size_t> membership( _num_verts );
    vector<size_t> degree( _num_verts );
    vector<unordered_set<size_t>> clusters(1);

    size_t source;
    size_t target;
    size_t weight;

    stack<size_t> cluster_ids;
    size_t max_id = 1;

    cluster_ids.push(max_id);

    while( 1 ) {

        scan_pos_int( edge_fp, source );
        scan_pos_int( edge_fp, target );
        scan_pos_int( edge_fp, weight );

        if ( weight == EOF ) break;

        degree[source] += 1;
        degree[target] += 1;

        int seen_1 = membership[source];
        int seen_2 = membership[target];

        if ( !seen_1 && !seen_2 ) {
            size_t cid = cluster_ids.top();
            cluster_ids.pop();
            membership[source] =  cid;
            membership[target] =  cid;
            if ( cid < max_id ) {
                clusters[cid].insert( source );
                clusters[cid].insert( target );
            } else {
                unordered_set<size_t> cluster;
                cluster.insert(source);
                cluster.insert(target);
                clusters.push_back( cluster );
                ++max_id;
                cluster_ids.push(max_id);
            }
        } else if ( seen_1 && !seen_2 ) {
            size_t cid = membership[source];
            membership[target] = cid;
            clusters[cid].insert( target );
        } else if ( !seen_1 && seen_2 ) {
            size_t cid = membership[target];
            membership[source] = cid;
            clusters[cid].insert( source );
        } else {
            size_t cid1 = membership[source];
            size_t cid2 = membership[target];
            if ( cid1 < cid2 ) {
                for ( size_t vert : clusters[cid2] ) {
                    membership[vert] = cid1;
                    clusters[cid1].insert(vert);
                }
                clusters[cid2].clear();
                cluster_ids.push(cid2);
            } else if ( cid2 < cid1 ) {
                for ( size_t vert : clusters[cid1] ) {
                    membership[vert] = cid2;
                    clusters[cid2].insert(vert);
                }
                clusters[cid1].clear();
                cluster_ids.push(cid1);
            }
        }
    }

    fclose( edge_fp );

    fprintf(stderr,"size cluster 0: %zd\n",clusters[0].size() );

    fprintf(stderr,"Calculating Fragmentation ...\n");

    double frag = 1.0;
    double nverts = static_cast<double>( _num_verts );
    double norm = 1.0/( nverts*( nverts - 1.0 ) );
    for ( unordered_set<size_t> cluster : clusters ) {
        double cs = static_cast<double>( cluster.size() );
        frag -= (cs*(cs-1.0)*norm);
    }

    fprintf(stderr,"Calculating Statisitcs ...\n");


    // some cluster statistics

    long num_clusters = 0;
    long max_cluster = 0;
    long num_nodes = 0;
    for ( unordered_set<size_t> cluster : clusters ) {
        long cs = cluster.size();
        if ( cs > 0 ) {
            if ( max_cluster < cs ) max_cluster = cs; 
            ++num_clusters;
            num_nodes += cs;
        }
    }
  
    double avg_cluster = static_cast<double>( num_nodes )/
                                    static_cast<double>( num_clusters );
    

    fprintf(stderr,"number of clusters: %zd\n", num_clusters);
    fprintf(stderr,"average cluster size: %10.3e\n", avg_cluster);
    fprintf(stderr,"max cluster size: %ld\n", max_cluster );
    fprintf(stderr,"number isolated vertices: %ld\n", 
                                            (_num_verts - num_nodes) );
    fprintf(stderr,"fragmentation: %14.7e\n",frag);

    fprintf(stderr,"Saving memberships ...\n");

    FILE *out_fp = fopen_csv( output_filename, "w" );

    fprintf( out_fp, "node\tmembership\n");
    size_t cluster_id = 1;
    for ( unordered_set<size_t> cluster : clusters ) {
        if ( cluster.size() > 0 ) {
            for ( size_t node : cluster ) {
                fprintf( out_fp, "%zd\t%zd\n", node, cluster_id );
            }
            ++cluster_id;
        }
    }
    fclose( out_fp );

}




void Graph::convert_list_to_mat( const string &edge_filename,
                                 const string &dc_filename,
                                 const string &evc_filename,
                                 const string &bucket_dir,
                                 const string &mat_filename ) {


    map_graph( edge_filename, dc_filename, evc_filename, bucket_dir );
    reduce_graph( bucket_dir, evc_filename, mat_filename );

}


void Graph::map_graph( const string &edge_filename,
                       const string &dc_filename,
                       const string &evc_filename,
                       const string &bucket_dir ) {

    size_t num_buckets = 251;

    vector<size_t> vert_degree( _num_verts );
    unordered_map<size_t,size_t> bucket_map;

    // Read in the node degrees
    fprintf(stderr,"Reading in node derees...\n");

    size_t vertex;
    size_t degree;

    FILE *dc_fp = fopen_csv( dc_filename, "r" );

    size_t num_edges = 0;

    while( 1 ) {
        scan_pos_int( dc_fp, vertex );
        scan_pos_int( dc_fp, degree );

        if ( degree == EOF ) break;

        vert_degree[vertex] = degree;

        num_edges += degree;
    }

    fclose( dc_fp );

    // Read in the eigenvector centralities
    fprintf(stderr,"Reading in evc...\n");

    FILE *evc_fp = fopen_csv( evc_filename, "r" );

    size_t rank;

    size_t bucket_size = num_edges/num_buckets + (num_edges % num_buckets);
    size_t current_bucket = 0;
    size_t contents = 0;

    while( 1 ) {

        scan_pos_int( evc_fp, vertex );
        scan_pos_int( evc_fp, rank );

        if ( rank == EOF ) break;

        bucket_map[vertex] = current_bucket;

        contents += vert_degree[vertex];
        
        if ( contents > bucket_size ) {
            ++current_bucket;
            contents = 0;
            if ( current_bucket >= num_buckets ) {
                fprintf( stderr, "not enough buckets!\n");
                abort();
            }
        }
    }

    fclose( evc_fp );

    // Create the buckets

    fprintf(stderr,"Creating buckets...\n");

    vector<string> bucket_filenames(num_buckets);
    string base = bucket_dir + "/bucket_";
    for ( size_t i = 0; i < num_buckets; ++i ) {
        if ( i < 10 ) {
            bucket_filenames[i] = base + "00" + std::to_string( i );
        } else if ( i < 100 ) {
            bucket_filenames[i] = base + "0" + std::to_string( i );
        } else {
            bucket_filenames[i] = base + std::to_string( i );
        }
    }

    vector<FILE*> buckets(num_buckets);
    for ( size_t i = 0; i < num_buckets; ++i ) {
        buckets[i] = fopen( bucket_filenames[i].c_str(), "w" );
    }


    // Process the edges

    fprintf(stderr,"Processing edges...\n");

    FILE *edge_fp = fopen_csv( edge_filename, "r" );

    size_t nodeA, nodeB, weight;

    while( 1 ) {

        scan_pos_int( edge_fp, nodeA );
        scan_pos_int( edge_fp, nodeB );
        scan_pos_int( edge_fp, weight );

        if ( weight == EOF ) break;

        size_t bucketA = bucket_map[nodeA];
        size_t bucketB = bucket_map[nodeB];

        //fprintf(stderr,"%zd %zd %zd\n",bucketA,bucketB,buckets.size());

        fprintf(buckets[bucketA],"%zd\t%zd\t%zd\n",nodeA,nodeB,weight);
        fprintf(buckets[bucketB],"%zd\t%zd\t%zd\n",nodeB,nodeA,weight);

    }

    fclose( edge_fp );

    fprintf(stderr,"Cleaning up ...\n");
    for ( size_t i = 0; i < num_buckets; ++i ) {
        fclose( buckets[i] );
    }

}
 
void Graph::reduce_graph( const string &bucket_dir,
                          const string &evc_filename,
                          const string &mat_filename ) {

    size_t num_buckets = 251;

    unordered_map<size_t,size_t> rank_map;

// Read in the eigenvector centralities
    fprintf(stderr,"Reading in evc...\n");

    FILE *evc_fp = fopen_csv( evc_filename, "r" );

    size_t vertex;
    size_t rank;

    while( 1 ) {

        scan_pos_int( evc_fp, vertex );
        scan_pos_int( evc_fp, rank );

        if ( rank == EOF ) break;

        rank_map[vertex] = rank;

    }

    fclose( evc_fp );


    fprintf(stderr,"Sorting buckets...\n");

    vector<string> bucket_filenames(num_buckets);
    string base = bucket_dir + "/bucket_";
    for ( size_t i = 0; i < num_buckets; ++i ) {
        if ( i < 10 ) {
            bucket_filenames[i] = base + "00" + std::to_string( i );
        } else if ( i < 100 ) {
            bucket_filenames[i] = base + "0" + std::to_string( i );
        } else {
            bucket_filenames[i] = base + std::to_string( i );
        }
    }

    fprintf(stderr,"Processing edges...\n");

    FILE *outfile = fopen_csv( mat_filename, "w" );

    MapCompare compare_rank( &rank_map );

    for ( size_t b = 0; b < num_buckets; ++b ) {

        fprintf(stderr,"... %s\n", bucket_filenames[b].c_str() );

        FILE *bucket = fopen_csv( bucket_filenames[b], "r", false );

        size_t nodeA, nodeB, weight;

        map<size_t,vector<pair<size_t,size_t>>,MapCompare> 
                                                    graph( compare_rank );

        while( 1 ) {

            scan_pos_int( bucket, nodeA );
            scan_pos_int( bucket, nodeB );
            scan_pos_int( bucket, weight );

            if ( weight == EOF ) break;

            graph[nodeA].push_back( pair<size_t,size_t>(nodeB, weight) );

        }

        fclose( bucket );

        map<size_t,vector<pair<size_t,size_t>>,MapCompare>::iterator git;
        for ( git = graph.begin(); git != graph.end(); ++git ) {
            fwrite( &(git->first), sizeof(size_t), 1, outfile );
            size_t nn = static_cast<size_t>( git->second.size() );

            sort( git->second.begin(), git->second.end(), compare_rank );

            fwrite( &nn, sizeof(size_t), 1, outfile );
            for ( pair<size_t,size_t> edge : git->second ) {
                fwrite( &(edge.first), sizeof(size_t), 1, outfile );
                fwrite( &(edge.second), sizeof(size_t), 1, outfile );
            }
        }

    }

    fclose( outfile );

    
}

