namespace leasy::app {
  bool should_exit = false;

  bool exit_requested() {
    return should_exit;
  }

  void request_exit() {
    should_exit = true;
  }
}