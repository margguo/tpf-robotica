#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>

#include <iostream>

struct indent {
  int depth_;
  indent(int depth): depth_(depth) {};
};

std::ostream & operator<<(std::ostream & o, indent const & in)
{
  for(int i = 0; i != in.depth_; ++i)
  {
    o << "  ";
  }
  return o;
}

int main(int /* argc */, char** /* argv */)
{
  // Set the global C and C++ locale to the user-configured locale,
  // so we can use std::cout with UTF-8, via Glib::ustring, without exceptions.
  std::locale::global(std::locale(""));

  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  try
  {
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
    xmlpp::TextReader reader("example.xml");

    while(reader.read())
    {
      int depth = reader.get_depth();
      std::cout << indent(depth) << "--- node ---" << std::endl;
      std::cout << indent(depth) << "name: " << reader.get_name() << std::endl;
      std::cout << indent(depth) << "depth: " << reader.get_depth() << std::endl;

      if(reader.has_attributes())
      {
        std::cout << indent(depth) << "attributes: " << std::endl;
        reader.move_to_first_attribute();
        do
        {
          std::cout << indent(depth) << "  " << reader.get_name() << ": " << reader.get_value() << std::endl;
        } while(reader.move_to_next_attribute());
        reader.move_to_element();
      }
      else
      {
        std::cout << indent(depth) << "no attributes" << std::endl;
      }

      if(reader.has_value())
        std::cout << indent(depth) << "value: '" << reader.get_value() << "'" << std::endl;
      else
        std::cout << indent(depth) << "novalue" << std::endl;

    }
  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  }
  catch(const std::exception& e)
  {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
}
