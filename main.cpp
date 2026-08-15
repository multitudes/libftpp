// #include "libftpp.hpp" // IWYU pragma: export
// #include "main_tests.hpp"
#include "test.hpp"
#include <iostream>

// =============================================================================
// Main Test
// =============================================================================
int main() {
  pool_test();
  std::cout << "\n\033[1;32m[✔] pool_test executed!\033[0m\n";
  databuffer_test();
  std::cout << "\n\033[1;32m[✔] databuffer_test executed!\033[0m\n";
  memento_test();
  std::cout << "\n\033[1;32m[✔] memento_test executed!\033[0m\n";
  observer_test();
  std::cout << "\n\033[1;32m[✔] observer_test executed!\033[0m\n";
  singleton_test();
  std::cout << "\n\033[1;32m[✔] singleton_test executed!\033[0m\n";
  // state_machine_test();
  // std::cout << "\n\033[1;32m[✔] state_machine_test executed!\033[0m\n";
  // thread_safe_iostream_test();
  // std::cout << "\n\033[1;32m[✔] thread_safe_iostream_test
  // executed!\033[0m\n";
  // thread_safe_queue_test();
  // std::cout << "\n\033[1;32m[✔] thread_safe_queue_test executed!\033[0m\n";
  // thread_test();
  // std::cout << "\n\033[1;32m[✔] thread_test executed!\033[0m\n";
  // workers_pool_test();
  // std::cout << "\n\033[1;32m[✔] workers_pool_test executed!\033[0m\n";
  // persistent_worker_test();
  // std::cout << "\n\033[1;32m[✔] persistent_worker_test executed!\033[0m\n";
  // message_test();
  // std::cout << "\n\033[1;32m[✔] message_test executed!\033[0m\n";
  // server_test();
  // std::cout << "\n\033[1;32m[✔] server_test executed!\033[0m\n";
  // ivector2_test();
  // std::cout << "\n\033[1;32m[✔] ivector2_test executed!\033[0m\n";
  // ivector3_test();
  // std::cout << "\n\033[1;32m[✔] ivector3_test executed!\033[0m\n";
  // random_2D_coordinate_generator_test();
  // std::cout << "\n\033[1;32m[✔] random_2D_coordinate_generator_test "
  //              "executed!\033[0m\n";
  // perlin_noise2D_test();
  // std::cout << "\n\033[1;32m[✔] perlin_noise2D_test executed!\033[0m\n";
  // ppm_image_exporter_test();
  // std::cout << "\n\033[1;32m[✔] ppm_image_exporter_test executed!\033[0m\n";
  // observable_value_test();
  // std::cout << "\n\033[1;32m[✔] observable_value_test executed!\033[0m\n";
  // timer_test();
  // std::cout << "\n\033[1;32m[✔] timer_test executed!\033[0m\n";
  // chronometer_test();
  // std::cout << "\n\033[1;32m[✔] chronometer_test executed!\033[0m\n";
  // command_pattern_test();
  // std::cout << "\n\033[1;32m[✔] command_pattern_test executed!\033[0m\n";
  // mixed_signature_test();
  // std::cout << "\n\033[1;32m[✔] mixed_signature_test executed!\033[0m\n";
  // lambda_command_test();
  // std::cout << "\n\033[1;32m[✔] mixed_signature_test executed!\033[0m\n";
  std::cout << "\n\n================================\n";
  std::cout << "============ END ===========\n";
  return 0;
}
