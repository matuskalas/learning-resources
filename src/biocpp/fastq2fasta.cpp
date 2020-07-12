#include <string_view>
#include <seqan3/io/sequence_file/all.hpp>

// Alias for a C-style string.
using character_string = char const *;

// argc: number of arguments. argv[]: list of arguments. N.B. The 1st argument is the name of the program/executable.
int main(int argc, character_string argv[])
{
    std::string_view fastq_file_path{argv[1]};  // Would work with (), but {} is newer & safer (memory stuff)!

    seqan3::sequence_file_output{std::cout, seqan3::format_fasta{}} =
        seqan3::sequence_file_input{fastq_file_path};
}
