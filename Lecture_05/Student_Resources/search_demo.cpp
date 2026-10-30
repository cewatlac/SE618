// SE618 Lecture 5 — tiny CLI over sample_docs.txt ("id|text" per line).
// g++ -std=c++20 -O1 search_demo.cpp -o demo && ./demo "posting lists" "cache lru"
#ifdef SOLUTION
#include "inverted_index_solution.hpp"
#else
#include "inverted_index.hpp"
#endif
#include <cstdio>
#include <fstream>

int main(int argc, char** argv) {
    se618::InvertedIndex ix;
    std::ifstream in("sample_docs.txt");
    for (std::string line; std::getline(in, line);) {
        auto bar = line.find('|');
        if (bar != std::string::npos) ix.add_document(static_cast<se618::DocId>(std::stoul(line.substr(0, bar))), line.substr(bar + 1));
    }
    std::printf("%zu documents, %zu terms\n", ix.size(), ix.vocabulary_size());
    for (int i = 1; i < argc; ++i) {
        std::printf("AND \"%s\":", argv[i]);
        for (auto d : ix.search_and(argv[i])) std::printf(" %u", d);
        std::printf("\n  ranked:");
        for (auto& [d, s] : ix.search_ranked(argv[i], 3)) std::printf(" %u(%.2f)", d, s);
        std::printf("\n");
    }
}
