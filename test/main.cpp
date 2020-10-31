#include <gtest/gtest.h>

#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>

extern "C" {
#include "thread_options.h"
#include "process_input.h"
#include "single_thread.h"
#include "multi_thread.h"
}

bool compareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
  std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);

  if (f1.fail() || f2.fail()) {
    return false;  //  file problem
  }

  if (f1.tellg() != f2.tellg()) {
    return false;  //  size mismatch
  }

  //  seek back to beginning and use std::equal to compare contents
  f1.seekg(0, std::ifstream::beg);
  f2.seekg(0, std::ifstream::beg);
  return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()));
}

const std::string TESTFILES_PATH = "../test/testfiles/";

TEST(ProcessInputTesting, test1) {
    std::string argvStrings[3] = {"test", "--thread=multi", "100"};
    char* argv[4];
    for (size_t i = 0; i < 3; ++i) {
        argv[i] = new char[argvStrings[i].size() + 1];
        strncpy(argv[i], argvStrings[i].c_str(), argvStrings[i].size() + 1);
    }
    argv[3] = nullptr;

    thread_options thread_option = SINGLE_THREAD;
    size_t array_len = 0;

    EXPECT_EQ(process_input(0, argv, &thread_option, &array_len), EXIT_FAILURE);
    EXPECT_EQ(process_input(3, NULL, &thread_option, &array_len), EXIT_FAILURE);
    EXPECT_EQ(process_input(3, argv, NULL, &array_len), EXIT_FAILURE);
    EXPECT_EQ(process_input(3, argv, &thread_option, NULL), EXIT_FAILURE);
    EXPECT_EQ(process_input(3, argv, &thread_option, &array_len), EXIT_SUCCESS);

    EXPECT_EQ(thread_option, MULTI_THREAD);
    EXPECT_EQ(array_len, 100);
    for (size_t i = 0; i < 3; ++i) {
        delete[] argv[i];
    }
}

TEST(SingleThreadTesting, test1) {
    std::ifstream infile(TESTFILES_PATH + "Test1in.txt");
    std::ofstream result(TESTFILES_PATH + "Test1out.txt");
    ASSERT_TRUE(infile && result) << "Could not open input"
                                  << std::endl;

    char* argv[4];
    int argc = 0;
    std::string temp = "";
    for (size_t i = 0; i < 3; ++i) {
        if (infile >> temp) {
            argv[i] = new char[temp.size() + 1];
            strncpy(argv[i], temp.c_str(), temp.size() + 1);
            ++argc;
        } else {
            break;
        }
    }
    argv[argc] = NULL;

    thread_options thread_option = SINGLE_THREAD;
    size_t array_len = 1 << 24;

    EXPECT_EQ(process_input(argc, argv, &thread_option, &array_len),
              EXIT_SUCCESS);
    for (size_t i = 0; i < argc; ++i) {
        delete[] argv[i];
    }

    int* array = new int[array_len];

    std::chrono::system_clock::duration start =
    std::chrono::system_clock::now().time_since_epoch();

    single_thread_fill(array, array_len);

    std::chrono::system_clock::duration end =
    std::chrono::system_clock::now().time_since_epoch();
    std::cout << "Filling time: "
              << std::fixed << std::setprecision(8)
              << static_cast<double>((end - start).count())
              << std::endl;

    for (size_t i = 0; i < array_len; ++i) {
        result << array[i] << ' ';
    }
    result << std::endl;

    delete[] array;
    infile.close();
    result.close();

    EXPECT_TRUE(compareFiles(TESTFILES_PATH + "Test1out.txt",
                             TESTFILES_PATH + "Test1expected.txt"))
               << "Output did not match expectations" << std::endl;
    remove((TESTFILES_PATH + "Test1out.txt").c_str());
}

TEST(MultiThreadTesting, test1) {
    std::ifstream infile(TESTFILES_PATH + "Test1in.txt");
    std::ofstream result(TESTFILES_PATH + "Test1out.txt");
    ASSERT_TRUE(infile && result) << "Could not open input"
                                  << std::endl;

    char* argv[4];
    int argc = 0;
    std::string temp = "";
    for (size_t i = 0; i < 3; ++i) {
        if (infile >> temp) {
            argv[i] = new char[temp.size() + 1];
            strncpy(argv[i], temp.c_str(), temp.size() + 1);
            ++argc;
        } else {
            break;
        }
    }
    argv[argc] = nullptr;

    thread_options thread_option = SINGLE_THREAD;
    size_t array_len = 1 << 24;

    EXPECT_EQ(process_input(argc, argv, &thread_option, &array_len),
              EXIT_SUCCESS);
    for (size_t i = 0; i < argc; ++i) {
        delete[] argv[i];
    }

    int* array = new int[array_len];

    std::chrono::system_clock::duration start =
    std::chrono::system_clock::now().time_since_epoch();

    multi_thread_fill(array, array_len);

    std::chrono::system_clock::duration end =
    std::chrono::system_clock::now().time_since_epoch();
    std::cout << "Filling time: "
              << std::fixed << std::setprecision(8)
              << static_cast<double>((end - start).count())
              << std::endl;

    for (size_t i = 0; i < array_len; ++i) {
        result << array[i] << ' ';
    }
    result << std::endl;

    delete[] array;
    infile.close();
    result.close();

    EXPECT_TRUE(compareFiles(TESTFILES_PATH + "Test1out.txt",
                             TESTFILES_PATH + "Test1expected.txt"))
               << "Output did not match expectations" << std::endl;
    remove((TESTFILES_PATH + "Test1out.txt").c_str());
}

TEST(ComparisonTests, test1) {
    size_t array_len = 1 << 24;
    int* arraySingle = new int[array_len];
    int* arrayMulti = new int[array_len];

    single_thread_fill(arraySingle, array_len);
    multi_thread_fill(arrayMulti, array_len);
    for (size_t i = 0; i < array_len; ++i) {
        EXPECT_EQ(arraySingle[i], arrayMulti[i]);
    }

    delete[] arraySingle;
    delete[] arrayMulti;
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
