function recursive_fibonacci(num) {
  if (num == 0) return 0;
  if (num == 1) return 1;
  return recursive_fibonacci(num - 2) + recursive_fibonacci(num - 1);
}

function tail_recursive_fibonacci(num) {
  if (num == 0) return 0;
  if (num == 1) return 1;
  return tail_recursive_fibonacci_impl(num, 0, 1);
}

function tail_recursive_fibonacci_impl(num, first, second) {
  var result = first + second;
  if (num == 2) return result;
  return tail_recursive_fibonacci_impl(num - 1, second, result);
}
