#include <stdexcept>    // Hmm, was added automatically?

#include <string_view>
#include <seqan3/std/ranges>    // To make it safe with ranges (C++20)

#include <seqan3/io/sequence_file/all.hpp>
#include <seqan3/alphabet/quality/all.hpp>  // Alphabet

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

// argc: number of arguments. argv[]: list of arguments. N.B. The 1st argument is the name of the program/executable.
int main(int argc, character_string argv[])
{
    std::string_view fastq_file_path{argv[1]};  // Would work with (), but {} is newer & safer (memory stuff)!
    std::string_view fasta_file_out_path{argv[2]};

    seqan3::sequence_file_output fasta_out{fasta_file_out_path};
    seqan3::sequence_file_input fastq_in{fastq_file_path};

    seqan3::phred42 minimal_quality = read_in_quality();    // phred42 score range 0..41

    for (auto && [seq, id, qual] : fastq_in)    // Cool modern C++: auto is type detection; && is refref to modifiable memory (just use "auto &&" & you're safe ;D); [,,] is a 3-tuple; fastq_in consists of 3-tuples.
    {
        if (std::ranges::all_of(qual, [&] (auto && quality) { return quality >= minimal_quality; }))   // Cool modern C++: Range-based algorithms (std::ranges::all_of() like in maths ALL OF). And lambda [&] () {} expression!
            fasta_out.emplace_back(seq, id);
    }
}
