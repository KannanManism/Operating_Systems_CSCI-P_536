#include <xinu.h>
// int work_queue_depth_copy, time_window_copy, output_time_copy,port_ID;

struct tscdf_element {
  int32 timestamp;
  int32 value;
  struct tscdf_element *vnext;
  struct tscdf_element *vprev;
};

struct tscdf {
  struct tscdf_element *data;
  struct tscdf_element *vhead;
  struct tscdf_element *vtail;
  int32 newest;
  int32 max_vals;
  int32 num_vals;
  sid32 mutex;
};

struct tscdf *
tscdf_init(int maxvals);

int32
tscdf_free(struct tscdf *tc);

int32
tscdf_update(struct tscdf *tc, int timestamp, int value);

int32 *
tscdf_walk(struct tscdf *tc);

int32 *
tscdf_quartiles(struct tscdf *tc);