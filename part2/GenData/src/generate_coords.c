#include "generate_coords.h"

#define CLUSTER_COUNT 64
#define CLUSTER_SIZE 10
#define EARTH_RADIUS 6372.8

typedef struct {
    f64 xmin, ymin;
    f64 xmax, ymax;
} Cluster;

typedef struct {
    f64 x0, y0;
    f64 x1, y1;
} Pair;

static f64 rand_f(f64 min, f64 max) {
    float x = (float)rand()/(float)(RAND_MAX/(max - min));
    x += min;
    return x;
}

static Pair generate_random_pair(Cluster cluster) {
    Pair pair = {
        rand_f(cluster.xmin, cluster.xmax),
        rand_f(cluster.ymin, cluster.ymax),
        rand_f(cluster.xmin, cluster.xmax),
        rand_f(cluster.ymin, cluster.ymax),
    };

    return pair;
}

static void print_json_header(FILE *flex_out) {
    fprintf(flex_out, "{\"pairs\":[\n");
}

static void print_json_footing(FILE *flex_out) {
    fprintf(flex_out, "]}");
}

static void print_json_pair(FILE *flex_out,
                            const Pair pair,
                            const bool is_last) {
    fprintf(flex_out, "\t{\"x0\":%f, \"y0\":%f, \"x1\":%f, \"y1\":%f}",
            pair.x0, pair.y0, pair.x1, pair.y1);
    if (!is_last)
        fprintf(flex_out, ",");
    fprintf(flex_out, "\n");
}

static void print_haversine(FILE *ans_out, 
                            const f64 haversine) {
    fwrite(&haversine, sizeof(haversine), 1, ans_out);
}

static f64 generate_coords_(FILE *flex_out,
                            FILE *ans_out,
                            const Cluster *clusters, 
                            const u32 cluster_amount,
                            const u32 n_pairs) {
    print_json_header(flex_out);

    f64 sum = 0;


    for (int i = 0; i < n_pairs; i++) {
        Cluster cluster = clusters[rand() % cluster_amount];
        Pair pair = generate_random_pair(cluster);
        f64 haversine = ReferenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1,
                                           EARTH_RADIUS);
        sum += haversine;
        print_json_pair(flex_out, pair, i == n_pairs - 1);
        print_haversine(ans_out, haversine);
    }
    
    print_json_footing(flex_out);
    print_haversine(ans_out, sum);

    return sum;
}

f64 generate_coords(FILE *flex_out,
                    FILE *ans_out,
                    const Method method,
                    const u32 seed,
                    const u32 n_pairs) {
    srand(seed);

    Cluster earth_bounds = {-180, -90, 180, 90}; 
    if (method == cluster) {
        Cluster cluster_center_bounds = {
            earth_bounds.xmin + CLUSTER_SIZE / 2,
            earth_bounds.ymin + CLUSTER_SIZE / 2,
            earth_bounds.xmax - CLUSTER_SIZE / 2,
            earth_bounds.ymax - CLUSTER_SIZE / 2,
        }; 

        Cluster clusters[CLUSTER_COUNT] = {};
        for (int i = 0; i < CLUSTER_COUNT; i++) {
            f64 cluster_center_x = rand_f(cluster_center_bounds.xmin,
                                          cluster_center_bounds.xmax);
            f64 cluster_center_y = rand_f(cluster_center_bounds.ymin,
                                          cluster_center_bounds.ymax);
            clusters[i] = (Cluster){
                cluster_center_x - CLUSTER_SIZE / 2,
                cluster_center_y - CLUSTER_SIZE / 2,
                cluster_center_x + CLUSTER_SIZE / 2,
                cluster_center_y + CLUSTER_SIZE / 2,
            };
        }

        return generate_coords_(flex_out, ans_out, clusters, CLUSTER_COUNT,
                                n_pairs);
    } else if (method == uniform) {
        Cluster earth_cluster[1] = {earth_bounds};
        return generate_coords_(flex_out, ans_out, earth_cluster, 1, n_pairs);
    }

    return 0;
}
