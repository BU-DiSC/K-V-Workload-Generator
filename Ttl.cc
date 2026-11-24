#include <vector>
#include <random>

int ttl_generator(long total_ttl_deletes) {
    static std::vector<int> choices = {88, 137, 275};
    
    static std::discrete_distribution<size_t> dist({0.1, 0.3, 0.6});

    static long generated = 0;
    long current = generated++;
    auto probabilities = dist.probabilities();
    double cumulative = 0;
    for (size_t i = 0; i + 1 < choices.size(); i++) {
        cumulative += probabilities[i];
        if (current < total_ttl_deletes * cumulative) {
            return choices[i];
        }
    }

    return choices.back();
}
