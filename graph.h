#ifndef GRAPH_H
#define GRAPH_H

#include <string>

class Graph {

 private:
    size_t _num_verts;
    size_t _num_edges;

 public:
    Graph( const size_t num_verts){
       _num_verts = num_verts; 
    };

    void degree_dist( const std::string &edge_filename, 
                      const std::string &output_filename );

    void eigen_vect_cent( const std::string &edge_filename, 
                          const std::string &output_filename,
                          const int num_it,
                          const double eps );

    void cluster_stats( const std::string &edge_filename, 
                        const std::string &output_filename );

    double modularity( const std::string &edge_filename,
                       const std::string &dc_filename,
                       const std::string &membership_filename );

    void convert_list_to_mat( const std::string &edge_filename,
                              const std::string &dc_filename,
                              const std::string &evc_filename,
                              const std::string &bucket_dir,
                              const std::string &mat_filename );

 private:

    void map_graph( const std::string &edge_filename,
                    const std::string &dc_filename,
                    const std::string &evc_filename,
                    const std::string &bucket_dir );
 
    void reduce_graph( const std::string &bucket_dir,
                       const std::string &evc_filename,
                       const std::string &mat_filename );
 

};

#endif // GRAPH_H
