#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

#include "libplatform/libplatform.h"
#include "v8.h"

template <int N>
struct template_fibonacci {
    enum { value = template_fibonacci<N-2>::value + template_fibonacci<N-1>::value };
};

template<>
struct template_fibonacci<0> {
    enum { value = 0 };
};

template<>
struct template_fibonacci<1> {
    enum { value = 1 };
};

int recursive_fibonacci(int num) {
    if (num == 0) return 0;
    if (num == 1) return 1;
    return recursive_fibonacci(num - 2) + recursive_fibonacci(num - 1);
}

int tail_recursive_fibonacci_impl(int, int, int);
int tail_recursive_fibonacci(int num) {
    if (num == 0) return 0;
    if (num == 1) return 1;
    return tail_recursive_fibonacci_impl(num, 0, 1);
}

int tail_recursive_fibonacci_impl(int num, int first, int second) {
    int result = first + second;
    if (num == 2) return result;
    return tail_recursive_fibonacci_impl(num - 1, second, result);
}

int loop_fibonacci(int num) {
    if (num == 0) return 0;
    if (num == 1) return 1;
    std::vector<int> nums(num+1);
    nums[0] = 0;
    nums[1] = 1;
    for (int i = 2; i < nums.size(); i++) {
        nums[i] = nums[i - 2] + nums[i - 1];
    }
    return nums.back();
}

std::string read_file(char const* path) {
    FILE* fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    std::string buf(size, 0);
    fread((void*)buf.data(), 1, size, fp);
    fclose(fp);
    return buf;
}

template<typename T>
void log(char const* name, unsigned int result, T duration) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    printf("%s\n", name);
    printf("result: %u\n",  result);
    printf("ms: %lld\n", ms);
    printf("us: %lld\n", us);
    printf("ns: %lld\n", ns);
}

int main(int argc, char* argv[]) {
    std::string input = read_file("input.js");
    char* num = getenv("FIBONACCI_NUM");
    input += "fibonacci(" + std::string(num) + ");";

    char const* builtin_path = getenv("BUILTIN_PATH");
    v8::V8::InitializeICUDefaultLocation(builtin_path);
    v8::V8::InitializeExternalStartupData(builtin_path);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(isolate, input.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();

        auto start_time = std::chrono::system_clock::now();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        auto end_time = std::chrono::system_clock::now();
        log("v8 fibonacci", result->Uint32Value(), end_time - start_time);
    }
    printf("\n");
    {
        auto start_time = std::chrono::system_clock::now();
        int result = recursive_fibonacci(atoi(num));
        auto end_time = std::chrono::system_clock::now();
        log("native recursive fibonacci", result, end_time - start_time);
    }
    printf("\n");
    {
        auto start_time = std::chrono::system_clock::now();
        int result = tail_recursive_fibonacci(atoi(num));
        auto end_time = std::chrono::system_clock::now();
        log("native tail recursive fibonacci", result, end_time - start_time);
    }
    printf("\n");
    {
        auto start_time = std::chrono::system_clock::now();
        int result = loop_fibonacci(atoi(num));
        auto end_time = std::chrono::system_clock::now();
        log("native loop fibonacci", result, end_time - start_time);
    }
#ifdef FIBONACCI_NUM
    printf("\n");
    {
        auto start_time = std::chrono::system_clock::now();
        int result = template_fibonacci<FIBONACCI_NUM>::value;
        auto end_time = std::chrono::system_clock::now();
        log("native template fibonacci", result, end_time - start_time);
    }
#endif

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
