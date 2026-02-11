#include <cstdio>
#include <cstring>
#include <gpiod.hpp>

enum class ProcessingNode
{
    Node1,
    Node2,
};

static ProcessingNode DetermineProcessingNode()
{
    gpiod::chip chip{"/dev/leopard/gpio-pn-id"};

    auto lineOffset = chip.get_line_offset_from_name("leopard-pn-id");

    gpiod::line_settings inputPin{};
    inputPin.set_direction(gpiod::line::direction::INPUT);
    inputPin.set_bias(gpiod::line::bias::DISABLED);
    inputPin.set_active_low(false);

    auto request = chip.prepare_request()
        .set_consumer("leopard-read-pn-id")
        .add_line_settings(lineOffset, inputPin)
        .do_request();

    auto value = request.get_value(lineOffset);

    if(value == gpiod::line::value::INACTIVE)
    {
        return ProcessingNode::Node1;
    }
    else
    {
        return ProcessingNode::Node2;
    }
}

struct FileDeleter
{
    void operator()(FILE* f)
    {
        fclose(f);
    }
};

using FileHandle = std::unique_ptr<FILE, FileDeleter>;

FileHandle OpenFile(const char* fname, const char* mode)
{
    auto f = fopen(fname, mode);

    if(f == nullptr)
    {
        throw std::system_error{};
    }

    return {f, {}};
}

static void WriteProcessingNodeId(FileHandle handle, ProcessingNode node)
{
    const char* name = node == ProcessingNode::Node1 ? "1" : "2";
    fwrite(name, 1, strlen(name), handle.get());
}

static void WriteProcessingNodeNetmask(FileHandle handle, ProcessingNode node)
{
    std::uint16_t netmask = node == ProcessingNode::Node1 ? 0x0080 : 0x0100;
    fwrite(reinterpret_cast<const char*>(&netmask), 1, sizeof(netmask), handle.get());
}

int main()
{
    auto node = DetermineProcessingNode();

    if (node == ProcessingNode::Node1)
    {
        puts("Running on PN1");
    }
    else
    {
        puts("Running on PN2");
    }

    WriteProcessingNodeId(OpenFile("/var/run/leopard-pn-id", "w"), node);
    WriteProcessingNodeNetmask(OpenFile("/var/run/leopard-pn-netmask", "wb"), node);

    return 0;
}