#include <string>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <omp.h>
#include <mutex>
#include <atomic>
#include <pthread.h>
// #include <chrono>   
// using namespace chrono;
using namespace std;

class segment
{
public:
    int type; // 0: 未设置, 1: 字母, 2: 数字, 3: 特殊字符
    int length; // 长度，例如S6的长度就是6
    segment(int type, int length)
    {
        this->type = type;
        this->length = length;
    };

    // 打印相关信息
    void PrintSeg();

    // 按照概率降序排列的value。例如，123是D3的一个具体value，其概率在D3的所有value中排名第三，那么其位置就是ordered_values[2]
    vector<string> ordered_values;

    // 按照概率降序排列的频数（概率）
    vector<int> ordered_freqs;

    // total_freq作为分母，用于计算每个value的概率
    int total_freq = 0;

    // 未排序的value，其中int就是对应的id
    unordered_map<string, int> values;

    // 根据id，在freqs中查找/修改一个value的频数
    unordered_map<int, int> freqs;


    void insert(string value);
    void order();
    void PrintValues();
};

class PT
{
public:



    // 例如，L6D1的content大小为2，content[0]为L6，content[1]为D1
    vector<segment> content;

    // pivot值，参见PCFG的原理
    int pivot = 0;
    void insert(segment seg);
    void PrintPT();

    // 导出新的PT
    vector<PT> NewPTs();

    // 记录当前每个segment（除了最后一个）对应的value，在模型中的下标
    vector<int> curr_indices;

    // 记录当前每个segment（除了最后一个）对应的value，在模型中的最大下标（即最大可以是max_indices[x]-1）
    vector<int> max_indices;
    // void init();
    float preterm_prob;
    float prob;
};

class model
{
public:
    // 对于PT/LDS而言，序号是递增的
    // 训练时每遇到一个新的PT/LDS，就获取一个新的序号，并且当前序号递增1
    int preterm_id = -1;
    int letters_id = -1;
    int digits_id = -1;
    int symbols_id = -1;
    int GetNextPretermID()
    {
        preterm_id++;
        return preterm_id;
    };
    int GetNextLettersID()
    {
        letters_id++;
        return letters_id;
    };
    int GetNextDigitsID()
    {
        digits_id++;
        return digits_id;
    };
    int GetNextSymbolsID()
    {
        symbols_id++;
        return symbols_id;
    };

    // C++上机和数据结构实验中，一般不允许使用stl
    // 这就导致大家对stl不甚熟悉。现在是时候体会stl的便捷之处了
    // unordered_map: 无序映射
    int total_preterm = 0;
    vector<PT> preterminals;
    int FindPT(PT pt);

    vector<segment> letters;
    vector<segment> digits;
    vector<segment> symbols;
    int FindLetter(segment seg);
    int FindDigit(segment seg);
    int FindSymbol(segment seg);

    unordered_map<int, int> preterm_freq;
    unordered_map<int, int> letters_freq;
    unordered_map<int, int> digits_freq;
    unordered_map<int, int> symbols_freq;

    vector<PT> ordered_pts;

    // 给定一个训练集，对模型进行训练
    void train(string train_path);

    // 对已经训练的模型进行保存
    void store(string store_path);

    // 从现有的模型文件中加载模型
    void load(string load_path);

    // 对一个给定的口令进行切分
    void parse(string pw);

    void order();

    // 打印模型
    void print();
};

// 优先队列，用于按照概率降序生成口令猜测
// 实际上，这个class负责队列维护、口令生成、结果存储的全部过程
class PriorityQueue
{
private:
    std::mutex mtx;  // 保护guesses和total_guesses
    int num_threads;

public:
    // 用vector实现的priority queue
    vector<PT> priority;

    // 模型作为成员，辅助猜测生成
    model m;

    // 计算一个pt的概率
    void CalProb(PT &pt);

    // 优先队列的初始化
    void init();

    // 对优先队列的一个PT，生成所有guesses
    void Generate(PT pt);

    // 将优先队列最前面的一个PT
    void PopNext();
    std::atomic<int> total_guesses{0};  // 使用atomic替代普通int
    vector<string> guesses;

    // 新增方法
    void setThreadCount(int count) {
        num_threads = count;
        omp_set_num_threads(count);
    }
};
//pthread适配的class：
/*class PriorityQueue
{
private:
    std::mutex mtx;  // 保护guesses和total_guesses
    int num_threads;
    const int MIN_TASK_SIZE = 10000;  // 最小任务大小阈值
    
    // 线程参数结构体
    struct ThreadArgs {
        PriorityQueue* pq;
        segment* seg;
        int start_idx;
        int end_idx;
        string prefix; }; // 用于第二个for循环的前缀
public:
    // 用vector实现的priority queue
    vector<PT> priority;

    // 模型作为成员，辅助猜测生成
    model m;

    // 计算一个pt的概率
    void CalProb(PT &pt);

    // 优先队列的初始化
    void init();

    // 对优先队列的一个PT，生成所有guesses
    void Generate(PT pt);

    // 将优先队列最前面的一个PT
    void PopNext();
    std::atomic<int> total_guesses{0};  // 使用atomic替代普通int
    vector<string> guesses;

    // 新增方法
    void setThreadCount(int count) {
        num_threads = count;
        omp_set_num_threads(count);

    }


    // 线程函数
    static void* thread_func(void* arg) {
        ThreadArgs* args = (ThreadArgs*)arg;
        PriorityQueue* pq = args->pq;
        segment* seg = args->seg;
        string prefix = args->prefix;
        
        vector<string> local_guesses;  // 本地存储,减少锁竞争
        
        for(int i = args->start_idx; i < args->end_idx; i++) {
            string guess = prefix + seg->ordered_values[i];
            local_guesses.push_back(guess);
        }
        
        // 将本地结果合并到全局结果
        {
            std::lock_guard<std::mutex> lock(pq->mtx);
            pq->guesses.insert(pq->guesses.end(), local_guesses.begin(), local_guesses.end());
            pq->total_guesses += local_guesses.size();
        }
        
        delete args;
        return nullptr;
    }

    // 串行处理函数
    void process_serial(segment* seg, int start_idx, int end_idx, const string& prefix) {
        for(int i = start_idx; i < end_idx; i++) {
            string guess = prefix + seg->ordered_values[i];
            guesses.push_back(guess);
            total_guesses++;
        }
    }

    // 并行处理函数
void process_parallel(segment* seg, int total_values, const string& prefix) {
    // 根据数据量动态计算最优线程数
    // 每个线程至少处理 MIN_TASK_SIZE 个任务
    int optimal_threads = std::min(num_threads, total_values / MIN_TASK_SIZE);
    
    // 确保至少有2个线程才进行并行处理
    if (optimal_threads < 2) {
        process_serial(seg, 0, total_values, prefix);
        return;
    }

    // 计算每个线程处理的数据量
    int chunk_size = (total_values + optimal_threads - 1) / optimal_threads;
    pthread_t* threads = new pthread_t[optimal_threads];
    
    // 创建线程
    for(int i = 0; i < optimal_threads; i++) {
        ThreadArgs* args = new ThreadArgs;
        args->pq = this;
        args->seg = seg;
        args->start_idx = i * chunk_size;
        args->end_idx = std::min((i + 1) * chunk_size, total_values);
        args->prefix = prefix;
        
        pthread_create(&threads[i], nullptr, thread_func, args);
    }
    
    // 等待所有线程完成
    for(int i = 0; i < optimal_threads; i++) {
        pthread_join(threads[i], nullptr);
    }
    
    delete[] threads;
} 
};*/