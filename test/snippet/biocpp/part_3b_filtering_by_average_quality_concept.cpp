#include <string_view>

#include <seqan3/alphabet/quality/all.hpp>
#include <seqan3/io/sequence_file/all.hpp>
#include <seqan3/range/views/to_rank.hpp>
#include <seqan3/std/ranges>

template <std::ranges::input_range range_t>
    requires std::integral<std::ranges::range_value_t<range_t>>
int32_t sum(range_t && range)
{
    int32_t intermediate_sum{};
    for (auto && element : range)
        intermediate_sum += element;

    return intermediate_sum;
}

// Request user interaction to provide the minimum base quality.
seqan3::phred42 read_user_base_quality()
{
    std::cout << "Please specify the minimum base quality as a probability between 0 and 41\n";
    int32_t user_base_quality{};
    std::cin >> user_base_quality;

    if (user_base_quality < 0 || user_base_quality > 41)
        throw std::invalid_argument{"Only values in the interval [0, 41] can be used."};

    return seqan3::phred42{}.assign_rank(user_base_quality);
}

using character_string = const char *;

int main(int const argc, character_string argv[])
{
#ifdef DATA_DIR
    (void) argc; (void) argv;

    std::string_view fastq_input_path{"" DATA_DIR "/SP1.fq"};
    std::string_view fasta_output_path{"" DATA_DIR "/SP1.solution3b.fa"};

    seqan3::phred42 minimum_phred_quality = seqan3::phred42{}.assign_rank(20);
#else
    if (argc != 3)
        return EXIT_FAILURE;

    std::string_view fastq_input_path{argv[1]};
    std::string_view fasta_output_path{argv[2]};

    seqan3::phred42 minimum_phred_quality = read_user_base_quality();
#endif

    seqan3::sequence_file_input seq_file_in{fastq_input_path};
    seqan3::sequence_file_output seq_file_out{fasta_output_path};

    // Only print sequences with average quality filter.
#if SEQAN3_WORKAROUND_GCC_93983
    auto seq_file_in_ = std::ranges::subrange{seq_file_in.begin(), seq_file_in.end()};
    seq_file_out = seq_file_in_
#else
    seq_file_out = seq_file_in
#endif
                 | std::views::filter([&] (auto && fastq_record)
                   {
                       auto quality_rank_view = seqan3::get<seqan3::field::qual>(fastq_record) | seqan3::views::to_rank;
                       seqan3::phred42 average_phred_quality = sum(quality_rank_view) / std::ranges::distance(quality_rank_view);
                       return average_phred_quality > minimum_phred_quality;
                   });

    return EXIT_SUCCESS;
}
