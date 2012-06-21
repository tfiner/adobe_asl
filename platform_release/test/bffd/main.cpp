/****************************************************************************************************/

#include <iostream>
#include <fstream>

#include <adobe/bffd/analyzer.hpp>
#include <adobe/bffd/parser.hpp>
#include <adobe/bffd/interface.hpp>

/****************************************************************************************************/

std::string get_input_line(std::istream& stream, std::streampos position)
{
    static std::vector<char> line_buffer(1024, 0);

    stream.clear();
    stream.seekg(position);
    stream.getline(&line_buffer[0], line_buffer.size());

    return std::string(&line_buffer[0], &line_buffer[static_cast<std::size_t>(stream.gcount())]);
}

/****************************************************************************************************/

void bffd_callback(std::istream&       /*stream*/,
                   inspection_branch_t /*top*/,
                   inspection_branch_t node)
{
    std::cerr << "found " << node->name_m
              << " @ " << node->start_offset_m << " to " << node->end_offset_m << '\n';
}

/****************************************************************************************************/

int main(int argc, char** argv)
try
{
    if (argc != 3)
        throw std::runtime_error("Usage: bffd <format description file> <binary file>");

    const char*     format_description_path(argv[1]);
    std::ifstream   format_description(format_description_path);
    std::ifstream   binary(argv[2]);
    bffd_analyzer_t analyzer(binary);

    try
    {
        adobe::line_position_t::getline_proc_t getline(new adobe::line_position_t::getline_proc_impl_t(boost::bind(&get_input_line, boost::ref(format_description), _2)));
        adobe::line_position_t                 position(adobe::name_t(format_description_path), getline);

        bffd_parser_t(format_description,
                      position,
                      boost::bind(&bffd_analyzer_t::set_current_structure, boost::ref(analyzer), _1),
                      boost::bind(&bffd_analyzer_t::add_field, boost::ref(analyzer), _1, _2),
                      boost::bind(&bffd_analyzer_t::add_const, boost::ref(analyzer), _1, _2),
                      boost::bind(&bffd_analyzer_t::add_invariant, boost::ref(analyzer), _1, _2),
                      boost::bind(&bffd_analyzer_t::add_notify, boost::ref(analyzer), _1),
                      boost::bind(&bffd_analyzer_t::add_skip, boost::ref(analyzer), _1, _2)).parse();
    }
    catch (const adobe::stream_error_t& error)
    {
        throw std::runtime_error(adobe::format_stream_error(format_description, error));
    }

    analyzer.monitor(&bffd_callback);
    analyzer.analyze_binary();

    bffd_interface_t interface(binary, analyzer.forest());

    interface.command_line();

    return 0;
}
catch (const user_quit_exception_t&)
{
    std::cout << "Goodbye." << std::endl;
    return 0;
}
catch (const std::exception& error)
{
    std::cerr << "Fatal error: " << error.what() << std::endl;
    return 1;
}
catch (...)
{
    std::cerr << "Fatal error: unknown" << std::endl;
    return 1;
}

/****************************************************************************************************/
