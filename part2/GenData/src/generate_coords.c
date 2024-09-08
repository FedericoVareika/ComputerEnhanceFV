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

#define U64Max UINT64_MAX

typedef struct 
{
    u64 A, B, C, D;
} random_series;

static u64 RotateLeft(u64 V, int Shift)
{
    u64 Result = ((V << Shift) | (V >> (64-Shift)));
    return Result;
}

static u64 RandomU64(random_series *Series)
{
    u64 A = Series->A;
    u64 B = Series->B;
    u64 C = Series->C;
    u64 D = Series->D;
    
    u64 E = A - RotateLeft(B, 27);
    
    A = (B ^ RotateLeft(C, 17));
    B = (C + D);
    C = (D + E);
    D = (E + A);
    
    Series->A = A;
    Series->B = B;
    Series->C = C;
    Series->D = D;
    
    return D;
}

static random_series Seed(u64 Value)
{
    random_series Series = {};
    
    // NOTE(casey): This is the seed pattern for JSF generators, as per the original post
    Series.A = 0xf1ea5eed;
    Series.B = Value;
    Series.C = Value;
    Series.D = Value;
    
    u32 Count = 20;
    while(Count--)
    {
        RandomU64(&Series);
    }
    
    return Series;
}

static f64 RandomInRange(random_series *Series, f64 Min, f64 Max)
{
    f64 t = (f64)RandomU64(Series) / (f64)U64Max;
    f64 Result = (1.0 - t)*Min + t*Max;
    
    return Result;
}

static f64 rand_f(f64 min, f64 max) {
    f64 x = (f64)rand()/(f64)(RAND_MAX);
    x = (1.0 - x)*min + x*max;
    return x;
}

static Pair generate_random_pair(random_series *Series, Cluster cluster) {
    Pair pair = {
        RandomInRange(Series, cluster.xmin, cluster.xmax),
        RandomInRange(Series, cluster.ymin, cluster.ymax),
        RandomInRange(Series, cluster.xmin, cluster.xmax),
        RandomInRange(Series, cluster.ymin, cluster.ymax),
    };

    return pair;
}

static void print_json_header(FILE *flex_out) {
    fprintf(flex_out, "{\"pairs\":[\n");
}

static void print_json_footing(FILE *flex_out) {
    fprintf(flex_out, "]}");
}

#ifdef DBL_DECIMAL_DIG
  #define OP_DBL_Digs (DBL_DECIMAL_DIG)
#else  
  #ifdef DECIMAL_DIG
    #define OP_DBL_Digs (DECIMAL_DIG)
  #else  
    #define OP_DBL_Digs (DBL_DIG + 3)
  #endif
#endif

static void print_json_pair(FILE *flex_out,
                            const Pair pair,
                            const bool is_last) {
    fprintf(flex_out, 
            "\t{"
            "\"x0\":%.17f, "
            "\"y0\":%.17f, "
            "\"x1\":%.17f, "
            "\"y1\":%.17f}",
            pair.x0,
            pair.y0, 
            pair.x1,
            pair.y1);
    if (!is_last)
        fprintf(flex_out, ",");
    fprintf(flex_out, "\n");
}

static void print_haversine(FILE *ans_out, 
                            const f64 haversine) {
    fwrite(&haversine, sizeof(f64), 1, ans_out);
}

static f64 generate_coords_(FILE *flex_out,
                            FILE *ans_out,
                            random_series *Series,
                            const Cluster *clusters, 
                            const u32 cluster_amount,
                            const u32 n_pairs) {
    print_json_header(flex_out);

    f64 sum = 0;


    for (int i = 0; i < n_pairs; i++) {
        Cluster cluster = clusters[rand() % cluster_amount];
        Pair pair = generate_random_pair(Series, cluster);
        f64 haversine = ReferenceHaversine(pair.x0, pair.y0, pair.x1, pair.y1,
                                           (f64)EARTH_RADIUS);
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
                    const u64 seed,
                    const u64 n_pairs) {
    random_series Series = Seed(seed);

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
            f64 cluster_center_x = RandomInRange(&Series, 
                                                 cluster_center_bounds.xmin,
                                                 cluster_center_bounds.xmax);
            f64 cluster_center_y = RandomInRange(&Series, 
                                                 cluster_center_bounds.ymin,
                                                 cluster_center_bounds.ymax);
            clusters[i] = (Cluster){
                cluster_center_x - CLUSTER_SIZE / 2,
                cluster_center_y - CLUSTER_SIZE / 2,
                cluster_center_x + CLUSTER_SIZE / 2,
                cluster_center_y + CLUSTER_SIZE / 2,
            };
        }

        return generate_coords_(flex_out, ans_out, &Series, clusters, CLUSTER_COUNT,
                                n_pairs);
    } else if (method == uniform) {
        Cluster earth_cluster[1] = {earth_bounds};
        return generate_coords_(flex_out, ans_out, &Series, earth_cluster, 1, n_pairs);
    }

    return 0;
}
