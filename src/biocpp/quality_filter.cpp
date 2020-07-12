// LOOK INTO /test/snippet/documentation (UPDATED IN THE UPSTREAM MASTER!) FOR COMMENTED LIVE CODE-ALONG EXERCISES!!!

// Count filtered sequences with e.g. grep '> SRR' SRR1770413_1_40k_filtered35avg.fasta | wc -l
// and original with e.g. grep '@SRR' SRR1770413_1_40k.fastq | wc -l

#include <stdexcept>    // Hmm, was added automatically?

#include <string_view>
#include <seqan3/std/ranges>    // To make it safe with ranges (C++20)
#include <seqan3/std/concepts>  // Ditto, explicitly declare a new C++20 feature

#include <seqan3/io/sequence_file/all.hpp>
#include <seqan3/alphabet/quality/all.hpp>  // Alphabet
#include <seqan3/range/views/to_rank.hpp>   // View that converts alphabet to rank


// Alias for a C-style string.
using character_string = char const *;


seqan3::phred42 read_in_quality()
{
    std::cout << "Please provide a minimal quality value (0 to 41):\n";
    int32_t user_quality{}; // {} 0-iniitalises the variable. We don't want uninitialised vars. Good to use int32_t here.
    std::cin >> user_quality;

    if (user_quality < 0 || user_quality > 41)
        throw std::invalid_argument{"Expected value between 0 and 41."};

    return seqan3::phred42{}.assign_rank(user_quality);
}


// Now define our own Concept
template <typename t>
concept semi_integral = requires (int32_t left_hand_side, t right_hand_side)
{
    { left_hand_side += right_hand_side };  // Require that += expression/assignment is valid.
    requires std::integral<decltype(left_hand_side += right_hand_side)>; // Require that the type returned (decltype) by += is integral.

    // Plus, remove_reference_t function on types. For me it, however, compiled also without it :/
};


template <std::ranges::input_range range_t> // Added Concept input_range for more safety (like a Java Interface, ++), with a lot of cool semantic features.
    requires std::integral<std::ranges::range_reference_t<range_t>> // Added Concept requirement that it's an integral type to allow +=. range_reference_t is a function working on types! :O
    // && foo<range_t>  // We could add more requirements.
int32_t sum(range_t && range)
{
    int32_t intermediate_sum{};
    for (auto && value : range)
        intermediate_sum += value;

    return intermediate_sum;
}


// argc: number of arguments. argv[]: list of arguments. N.B. The 1st argument is the name of the program/executable.
int main(int argc, character_string argv[])
{
    std::string_view fastq_file_path{argv[1]};  // Would work with (), but {} is newer & safer (memory stuff)!
    std::string_view fasta_file_out_path{argv[2]};

    seqan3::sequence_file_output fasta_out{fasta_file_out_path};
    seqan3::sequence_file_input fastq_in{fastq_file_path};

    seqan3::phred42 minimal_quality = read_in_quality();    // phred42 score range 0..41

    // Second version with averaging quality scores and then filtering
    fasta_out = fastq_in | std::views::filter([&] (auto && fastq_record)     // Fancy C++20 Views. Plus [] () {} lambdex -- fails here with [] while knowing that we probably wanted to capture minimal_quality -- use [&] to capture it.
    {
        auto rank_quality_view = seqan3::get<seqan3::field::qual>(fastq_record)
                               | seqan3::views::to_rank;    // Piping it further. The disputable indent used so by the presenter (René Rahn)
        seqan3::phred42 average_quality = sum(rank_quality_view) /
                                          std::ranges::distance(rank_quality_view);
        return average_quality >= minimal_quality;
    });

    // First version was conservative, with all quality scores of a sequence above the threshold.
    // for (auto && [seq, id, qual] : fastq_in)    // Cool modern C++: auto is type detection; && is refref to modifiable memory (just use "auto &&" & you're safe ;D); [,,] is a 3-tuple; fastq_in consists of 3-tuples.
    // {
    //     if (std::ranges::all_of(qual, [&] (auto && quality) { return quality >= minimal_quality; }))   // Cool modern C++: Range-based algorithms (std::ranges::all_of() like in maths ALL OF). And lambda [&] () {} expression!
    //         fasta_out.emplace_back(seq, id);
    // }
}
