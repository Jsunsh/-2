#include <iostream>
#include <chrono>
using namespace std;
using namespace std::chrono;
int normal(int* arr,int n){
   int sum=0;
   for(int i=0;i<n;i++){
    sum+=arr[i];
   }
   return sum;
}
int add1(int* a,int n){
    int sum1=0,sum2=0;
    int i=0;
    for(;i+1<n;i+=2){
        sum1+=a[i];
        sum2+=a[i+1];
    }
    for(;i<n;i++){
        sum1+=a[i];

    }
    return sum1+sum2;
}
int sum_digui(int* arr, int n) {
    if (n == 1) return arr[0];          // 递归基：只剩1个元素
    int mid = n / 2;                    // 将数组分成两半
    return sum_digui(arr, mid) +    // 递归计算左半部分
           sum_digui(arr + mid, n - mid); // 递归计算右半部分
}
//测试

void run_test(int* data, int n, const char* name) {
    const int iterations = 10000;
    volatile int dummy = 0;
    int* dynamic_data = new int[n];

    // 初始化动态数据（防止编译器优化）
    for (int j = 0; j < n; j++) {
        dynamic_data[j] = data[j];
    }

    // 测试普通版本
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        dynamic_data[0] = i % 3;  // 微调输入数据
        dummy += normal(dynamic_data, n);
    }
    auto normal_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();

    // 测试优化版本
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        dynamic_data[0] = i % 3;
        dummy += add1(dynamic_data, n);
    }
    auto opt_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();

    // 测试递归版本（新增部分）
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        dynamic_data[0] = i % 3;
        dummy += sum_digui(dynamic_data, n);
    }
    auto rec_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();

    // 输出结果
    cout << "=== " << name << " (n=" << n << ") ===" << endl;
    cout << "Normal:    " << normal_time / iterations << " ns/call" << endl;
    cout << "Optimized: " << opt_time / iterations << " ns/call" << endl;
    cout << "Recursive: " << rec_time / iterations << " ns/call" << endl;

    // 计算速度比
    if (opt_time > 0) {
        cout << "Speed ratio (Normal/Opt): " << (double)normal_time / opt_time << "x" << endl;
    }
    if (rec_time > 0) {
        cout << "Speed ratio (Normal/Rec): " << (double)normal_time / rec_time << "x" << endl;
    }
    cout << endl;

    delete[] dynamic_data;
}

int main()
{// 固定测试数据集
  int TEST_DATA[10000]={};
  for (int i=0;i<10000;i++){
    TEST_DATA[i]=i+1;
  }
  run_test(TEST_DATA,10000, "10000个数");


    return 0;
}
