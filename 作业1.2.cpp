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
    if (n == 1) return arr[0];          // �ݹ����ֻʣ1��Ԫ��
    int mid = n / 2;                    // ������ֳ�����
    return sum_digui(arr, mid) +    // �ݹ������벿��
           sum_digui(arr + mid, n - mid); // �ݹ�����Ұ벿��
}
//����

void run_test(int* data, int n, const char* name) {
    const int iterations = 10000;
    volatile int dummy = 0;
    int* dynamic_data = new int[n];

    // ��ʼ����̬���ݣ���ֹ�������Ż���
    for (int j = 0; j < n; j++) {
        dynamic_data[j] = data[j];
    }

    // ������ͨ�汾
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        dynamic_data[0] = i % 3;  // ΢����������
        dummy += normal(dynamic_data, n);
    }
    auto normal_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();

    // �����Ż��汾
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        dynamic_data[0] = i % 3;
        dummy += add1(dynamic_data, n);
    }
    auto opt_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();

    // ���Եݹ�汾���������֣�
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        dynamic_data[0] = i % 3;
        dummy += sum_digui(dynamic_data, n);
    }
    auto rec_time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();

    // ������
    cout << "=== " << name << " (n=" << n << ") ===" << endl;
    cout << "Normal:    " << normal_time / iterations << " ns/call" << endl;
    cout << "Optimized: " << opt_time / iterations << " ns/call" << endl;
    cout << "Recursive: " << rec_time / iterations << " ns/call" << endl;

    // �����ٶȱ�
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
{// �̶��������ݼ�
  int TEST_DATA[10000]={};
  for (int i=0;i<10000;i++){
    TEST_DATA[i]=i+1;
  }
  run_test(TEST_DATA,10000, "10000����");


    return 0;
}
