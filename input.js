function fibonacci(num) {
  if (num == 0) return 0;
  if (num == 1) return 1;
  return fibonacci(num - 2) + fibonacci(num - 1);
}
