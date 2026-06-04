/*
 * 流敏感性决策规则完整测试用例（改进版）
 * 确保覆盖所有flow-sensitive和flow-insensitive的情况
 * 
 * 目标：
 * - 规则0触发 → flow-insensitive
 * - 规则1/2/3触发 → flow-sensitive
 * - 规则4综合评分低 → flow-insensitive
 * - 规则4综合评分高 → flow-sensitive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 组1: 明确的 FLOW-INSENSITIVE 测试用例
// ============================================================================

/**
 * 测试1: 纯计算函数，无指针操作
 * 预期: flow-insensitive（规则0或规则4低分）
 */
int simple_add(int a, int b) {
    return a + b;
}

/**
 * 测试2: 简单的max函数
 * 预期: flow-insensitive
 */
int simple_max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * 测试3: 简单的三目运算
 * 预期: flow-insensitive
 */
int simple_abs(int x) {
    return (x < 0) ? -x : x;
}

/**
 * 测试4: 单个store操作
 * 预期: flow-insensitive（规则0: storeCount < 3）
 */
void single_store(int* ptr) {
    *ptr = 42;
}

/**
 * 测试5: 两个store操作
 * 预期: flow-insensitive（规则0: storeCount < 3）
 */
void double_store(int* a, int* b) {
    *a = 1;
    *b = 2;
}

/**
 * 测试6: 简单的条件判断（低分支，低store）
 * 预期: flow-insensitive（规则4: 综合评分低）
 */
int simple_check(int x) {
    if (x > 0) {
        return 1;
    }
    return 0;
}

/**
 * 测试7: 简单的计算函数（无循环，无复杂分支）
 * 预期: flow-insensitive
 */
int calculate_average(int a, int b, int c) {
    int sum = a + b + c;
    return sum / 3;
}

/**
 * 测试8: 只读函数（只有load，没有store）
 * 预期: flow-insensitive
 */
int read_only_sum(int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size && i < 10; i++) {
        sum += arr[i];  // 只有load，没有store到arr
    }
    return sum;
}

// ============================================================================
// 组2: 明确的 FLOW-SENSITIVE 测试用例（规则1: Store密度）
// ============================================================================

/**
 * 测试9: 高store密度 - Buffer填充
 * 预期: flow-sensitive（规则1: store密度 > 15%）
 */
void high_store_buffer(char* buf) {
    buf[0] = 'A';
    buf[1] = 'B';
    buf[2] = 'C';
    buf[3] = 'D';
    buf[4] = 'E';
    buf[5] = 'F';
    buf[6] = 'G';
    buf[7] = 'H';
    buf[8] = 'I';
    buf[9] = 'J';
    buf[10] = '\0';
}

/**
 * 测试10: 高store密度 - 数组初始化
 * 预期: flow-sensitive（规则1）
 */
void high_store_array(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
        arr[i] = arr[i] + 1;  // 重复赋值，flow-insensitive会混淆
    }
}

/**
 * 测试11: 边界store密度（15%附近）
 * 预期: T=0.5时flow-sensitive，T=0.2时flow-insensitive
 */
void boundary_store_density(int* a, int* b, int* c) {
    *a = 10;
    *b = 20;
    *c = 30;
    // 约15% store密度
    int x = *a + *b;
    int y = x * 2;
    int z = y + *c;
}

// ============================================================================
// 组3: 明确的 FLOW-SENSITIVE 测试用例（规则2: Branch复杂度）
// ============================================================================

/**
 * 测试12: 高分支数量（> 5个分支）
 * 预期: flow-sensitive（规则2: branch > 5）
 */
int high_branch_validator(int value) {
    int result = value;
    
    if (value < 0) result = 0;           // branch 1
    if (value > 100) result = 100;       // branch 2
    if (value == 50) result = 75;        // branch 3
    if (value % 2 == 0) result += 1;     // branch 4
    if (value % 3 == 0) result += 2;     // branch 5
    if (value % 5 == 0) result += 5;     // branch 6
    
    return result;
}

/**
 * 测试13: 非常高的分支数量
 * 预期: flow-sensitive（规则2）
 */
void very_high_branch(int* ptr, int val) {
    int* result = ptr;
    int dummy = 0;
    
    if (val < 0) result = &dummy;        // branch 1
    if (val > 100) result = &dummy;      // branch 2
    if (val == 50) result = ptr + 1;     // branch 3
    if (val % 2 == 0) result = ptr + 2;  // branch 4
    if (val % 3 == 0) result = ptr - 1;  // branch 5
    if (val % 5 == 0) result = ptr;      // branch 6
    if (val % 7 == 0) result = ptr + 3;  // branch 7
    if (val % 11 == 0) result = &dummy;  // branch 8
    
    *result = val;
}

/**
 * 测试14: 边界分支数量（5个分支）
 * 预期: T=0.5时flow-sensitive，T=0.2时flow-insensitive
 */
int boundary_branch_count(int x) {
    int r = x;
    if (x < 0) r = 0;      // branch 1
    if (x > 100) r = 100;  // branch 2
    if (x == 50) r = 75;   // branch 3
    if (x % 2 == 0) r++;   // branch 4
    if (x % 3 == 0) r++;   // branch 5
    return r;
}

// ============================================================================
// 组4: FLOW-SENSITIVE（规则3: Sparse分析收益）
// ============================================================================

/**
 * 测试15: 低dataFlow密度（纯计算）
 * 预期: flow-sensitive（规则3: dataFlowDensity < T3）
 * 注意：在LLVM IR层面可能不如预期那样低
 */
long sparse_friendly_compute(int a, int b, int c) {
    int x1 = a + b;
    int x2 = b + c;
    int x3 = a + c;
    int x4 = x1 * x2;
    int x5 = x2 * x3;
    int x6 = x1 * x3;
    long result = (long)x4 + (long)x5 + (long)x6;
    return result;
}

// ============================================================================
// 组5: 综合评分测试（规则4）
// ============================================================================

/**
 * 测试16: 综合评分高（有循环）
 * 预期: flow-sensitive（规则4: score高，循环因子放大）
 */
void composite_high_score_loop(int* arr, int size) {
    for (int i = 0; i < size; i++) {      // 循环
        if (arr[i] < 0) {                 // 分支
            arr[i] = 0;                   // store
        } else if (arr[i] > 100) {        // 分支
            arr[i] = 100;                 // store
        }
    }
}

/**
 * 测试17: 综合评分低（无循环，分支少，store少）
 * 预期: flow-insensitive（规则4: score低）
 */
int composite_low_score(int a, int b) {
    if (a > b) {
        return a - b;
    }
    return b - a;
}

/**
 * 测试18: 综合评分中等（边界情况）
 * 预期: 根据tolerance决定
 */
void composite_moderate_score(int* data, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] > 0) {
            count++;
        }
    }
    if (count > n / 2) {
        data[0] = count;
    }
}

// ============================================================================
// 组6: Tolerance敏感度测试
// ============================================================================

/**
 * 测试19: Store密度14%（略低于15%阈值）
 * 预期: T=0.2时flow-insensitive，T=0.5时flow-insensitive，T=0.8时flow-sensitive
 */
void tolerance_test_store_14(int* arr) {
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    // 设计为约14% store密度
    int a = arr[0] + arr[1];
    int b = arr[1] + arr[2];
    int c = a * b;
    int d = c / 2;
}

/**
 * 测试20: Store密度16%（略高于15%阈值）
 * 预期: T=0.5时flow-sensitive，T=0.2时可能flow-insensitive
 */
void tolerance_test_store_16(int* arr) {
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    // 设计为约16% store密度
    int sum = arr[0] + arr[1] + arr[2] + arr[3];
}

// ============================================================================
// 组7: 特殊场景
// ============================================================================

/**
 * 测试21: 递归函数（简单）
 * 预期: 根据其他特征决定
 */
int factorial_simple(int n) {
    if (n <= 1) return 1;
    return n * factorial_simple(n - 1);
}

/**
 * 测试22: 函数调用密集
 * 预期: 根据其他特征决定（callCount不直接影响flow sensitivity）
 */
void call_intensive(int* arr, int size) {
    memset(arr, 0, size * sizeof(int));
    for (int i = 0; i < size; i++) {
        arr[i] = abs(arr[i]);
    }
}

/**
 * 测试23: 空函数
 * 预期: flow-insensitive（规则0）
 */
void empty_function(void) {
    // 什么都不做
}

/**
 * 测试24: 只有return的函数
 * 预期: flow-insensitive（规则0）
 */
int just_return_zero(void) {
    return 0;
}

/**
 * 测试25: 只有return常量
 * 预期: flow-insensitive
 */
int return_constant(void) {
    return 42;
}

// ============================================================================
// Main函数
// ============================================================================

int main() {
    printf("=== 流敏感性完整测试 ===\n\n");
    
    // 组1: flow-insensitive测试
    printf("组1: flow-insensitive预期\n");
    printf("  simple_add: %d\n", simple_add(1, 2));
    printf("  simple_max: %d\n", simple_max(5, 3));
    printf("  simple_abs: %d\n", simple_abs(-10));
    
    int x = 0;
    single_store(&x);
    printf("  single_store: %d\n", x);
    
    int a, b;
    double_store(&a, &b);
    printf("  double_store: %d, %d\n", a, b);
    
    // 组2: flow-sensitive（store密度）
    printf("\n组2: flow-sensitive（store密度）预期\n");
    char buf[20];
    high_store_buffer(buf);
    printf("  high_store_buffer: %s\n", buf);
    
    int arr[10];
    high_store_array(arr, 10);
    printf("  high_store_array: done\n");
    
    // 组3: flow-sensitive（branch复杂度）
    printf("\n组3: flow-sensitive（branch复杂度）预期\n");
    printf("  high_branch_validator: %d\n", high_branch_validator(42));
    
    // 组4: flow-sensitive（sparse收益）
    printf("\n组4: flow-sensitive（sparse收益）预期\n");
    printf("  sparse_friendly_compute: %ld\n", sparse_friendly_compute(1, 2, 3));
    
    // 组5: 综合评分
    printf("\n组5: 综合评分测试\n");
    composite_high_score_loop(arr, 10);
    printf("  composite_high_score_loop: done\n");
    printf("  composite_low_score: %d\n", composite_low_score(5, 3));
    
    // 组6: Tolerance敏感度
    printf("\n组6: Tolerance敏感度测试\n");
    tolerance_test_store_14(arr);
    tolerance_test_store_16(arr);
    printf("  tolerance tests: done\n");
    
    // 组7: 特殊场景
    printf("\n组7: 特殊场景\n");
    printf("  factorial_simple: %d\n", factorial_simple(5));
    printf("  empty_function: done\n");
    empty_function();
    printf("  just_return_zero: %d\n", just_return_zero());
    printf("  return_constant: %d\n", return_constant());
    
    printf("\n=== 测试完成 ===\n");
    return 0;
}
