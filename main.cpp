#include "libftpp.hpp" // IWYU pragma: export
#include "main_tests.hpp"
#include <iostream>

// =============================================================================
// Main Test
// =============================================================================
int main() {
  // pool_test();
  // databuffer_test();
  // memento_test();
  // observer_test();
  // singleton_test();
  // state_machine_test();
  // thread_safe_iostream_test();
  // thread_safe_queue_test();
  // thread_test();
  // workers_pool_test();
  // persistent_worker_test();
  // message_test();
  // server_test();
  // ivector2_test();
  // ivector3_test();
  // random_2D_coordinate_generator_test();
  // perlin_noise2D_test();
  // ppm_image_exporter_test();
  // observable_value_test();
  timer_test();
  chronometer_test();
  command_pattern_test();
  mixed_signature_test();
  std::cout << "\n\n================================\n";
  std::cout << "============ END ===========\n";
  return 0;
}
