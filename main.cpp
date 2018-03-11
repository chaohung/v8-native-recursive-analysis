#include <chrono>
#include <cstdio>
#include <string>

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
        auto duration = end_time - start_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        printf("v8 fibonacci\n");
        printf("result: %u\n",  result->Uint32Value());
        printf("ms: %lld\n", ms);
        printf("us: %lld\n", us);
        printf("ns: %lld\n", ns);
    }
    printf("\n");
    {
        auto start_time = std::chrono::system_clock::now();
        int result = recursive_fibonacci(atoi(num));
        auto end_time = std::chrono::system_clock::now();
        auto duration = end_time - start_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        printf("native recursive fibonacci\n");
        printf("result: %u\n",  result);
        printf("ms: %lld\n", ms);
        printf("us: %lld\n", us);
        printf("ns: %lld\n", ns);
    }
#ifdef FIBONACCI_NUM
    printf("\n");
    {
        auto start_time = std::chrono::system_clock::now();
        int result = template_fibonacci<FIBONACCI_NUM>::value;
        auto end_time = std::chrono::system_clock::now();
        auto duration = end_time - start_time;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        printf("native template fibonacci\n");
        printf("result: %u\n",  result);
        printf("ms: %lld\n", ms);
        printf("us: %lld\n", us);
        printf("ns: %lld\n", ns);
    }
#endif

    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
