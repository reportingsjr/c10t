#ifndef _NBT_H_
#define _NBT_H_

#include <string>
#include <stdint.h>
#include <zlib.h>
#include <stack>
#include <list>

#include <iostream>
#include <string.h>
#include <exception>

namespace nbt {
  class bad_grammar : std::exception {};
  #define BUFFER_SIZE 1024
  
  typedef int8_t Byte;
  typedef int16_t Short;
  typedef int32_t Int;
  typedef int64_t Long;
  typedef std::string String;
  typedef float Float;
  typedef double Double;
  
  struct ByteArray {
    Int length;
    Byte *values;
    ~ByteArray() {
      delete [] values;
    }
  };
  
  const Byte TAG_End = 0x0;
  const Byte TAG_Byte = 0x1;
  const Byte TAG_Short = 0x2;
  const Byte TAG_Int = 0x3;
  const Byte TAG_Long = 0x4;
  const Byte TAG_Float = 0x5;
  const Byte TAG_Double = 0x6;
  const Byte TAG_Byte_Array = 0x7;
  const Byte TAG_String = 0x8;
  const Byte TAG_List = 0x9;
  const Byte TAG_Compound = 0xa;
  
  const std::string TAG_End_str("TAG_End");
  const std::string TAG_Byte_str("TAG_Byte");
  const std::string TAG_Short_str("TAG_Short");
  const std::string TAG_Int_str("TAG_Int");
  const std::string TAG_Long_str("TAG_Long");
  const std::string TAG_Float_str("TAG_Float");
  const std::string TAG_Double_str("TAG_Double");
  const std::string TAG_Byte_Array_str("TAG_Byte_Array");
  const std::string TAG_String_str("TAG_String");
  const std::string TAG_List_str("TAG_List");
  const std::string TAG_Compound_str("TAG_Compound");
  
  const std::string tag_string_map[] = {
    TAG_End_str,
    TAG_Byte_str,
    TAG_Short_str,
    TAG_Int_str,
    TAG_Long_str,
    TAG_Float_str,
    TAG_Double_str,
    TAG_Byte_Array_str,
    TAG_String_str,
    TAG_List_str,
    TAG_Compound_str
  };
  
  typedef void (*begin_compound_t)(void *context, String name);
  typedef void (*end_compound_t)(void *context);
  
  typedef void (*begin_list_t)(void *context, String name, Byte type, Int length);
  typedef void (*end_list_t)(void *context);
  
  typedef void (*register_long_t)(void *context, String name, Long l);
  typedef void (*register_short_t)(void *context, String name, Short l);
  typedef void (*register_string_t)(void *context, String name, String l);
  typedef void (*register_float_t)(void *context, String name, Float l);
  typedef void (*register_double_t)(void *context, String name, Double l);
  typedef void (*register_int_t)(void *context, String name, Int l);
  typedef void (*register_byte_t)(void *context, String name, Byte b);
  typedef void (*register_byte_array_t)(void *context, String name, ByteArray *array);
  typedef void (*error_handler_t)(void *context, size_t where, const char *why);
  
  void default_begin_compound(void *context, nbt::String name);
  void default_end_compound(void *context);
  void default_begin_list(void *context, nbt::String name, nbt::Byte type, nbt::Int length);
  void default_end_list(void *context);
  void default_error_handler(void *context, size_t where, const char *why);
  
  bool is_big_endian();
  
  class Parser {
    private:
      void *context;

      void assert_error(gzFile file, bool a, const char *why) {
        if (!a) {
          size_t where = file == NULL ? 0 : gztell(file);
          error_handler(context, where, why);
          throw bad_grammar();
        }
      }

      Byte read_byte(gzFile file) {
        Byte b;
        assert_error(file, gzread(file, &b, sizeof(Byte)) == sizeof(Byte), "Buffer too short to read Byte");
        return b;
      }
      
      Short read_short(gzFile file) {
        uint8_t b[2];
        assert_error(file, gzread(file, b, sizeof(b)) == sizeof(b), "Buffer to short to read Short");
        Short s = (b[0] << 8) + b[1];
        return s;
      }

      Int read_int(gzFile file) {
        Int i;
        Byte b[sizeof(i)];
        assert_error(file, gzread(file, b, sizeof(b)) == sizeof(b), "Buffer to short to read Int");
        Int *ip = &i;
        
        if (is_big_endian()) {
          *((Byte*)ip) = b[0];
          *((Byte*)ip + 1) = b[1];
          *((Byte*)ip + 2) = b[2];
          *((Byte*)ip + 3) = b[3];
        } else {
          *((Byte*)ip) = b[3];
          *((Byte*)ip + 1) = b[2];
          *((Byte*)ip + 2) = b[1];
          *((Byte*)ip + 3) = b[0];
        }
        
        return i;
      }
      
      String read_string(gzFile file) {
        Short s = read_short(file);
        uint8_t *str = new uint8_t[s + 1];
        assert_error(file, gzread(file, str, s) == s, "Buffer to short to read String");
        String so((const char*)str, s);
        delete [] str;
        return so;
      }
      
      void flush_string(gzFile file) {
        Short s = read_short(file);
        assert_error(file, gzseek(file, s, SEEK_CUR) != -1, "Buffer to short to flush String");
      }
      
      Float read_float(gzFile file)
      {
        Float f;
        Byte b[sizeof(f)];
        assert_error(file, gzread(file, b, sizeof(f)) == sizeof(f), "Buffer to short to read Float");
        Float *fp = &f;
        
        if (is_big_endian()) {
          *((Byte*)fp) = b[0];
          *((Byte*)fp + 1) = b[1];
          *((Byte*)fp + 2) = b[2];
          *((Byte*)fp + 3) = b[3];
        } else {
          *((Byte*)fp) = b[3];
          *((Byte*)fp + 1) = b[2];
          *((Byte*)fp + 2) = b[1];
          *((Byte*)fp + 3) = b[0];
        }
        
        return f;
      }
      
      Long read_long(gzFile file) {
        Long l;
        Byte b[sizeof(l)];
        assert_error(file, gzread(file, b, sizeof(b)) == sizeof(b), "Buffer to short to read Long");
        Long *lp = &l;
        
        if (is_big_endian()) {
          *((Byte*)lp) = b[0];
          *((Byte*)lp + 1) = b[1];
          *((Byte*)lp + 2) = b[2];
          *((Byte*)lp + 3) = b[3];
          *((Byte*)lp + 4) = b[4];
          *((Byte*)lp + 5) = b[5];
          *((Byte*)lp + 6) = b[6];
          *((Byte*)lp + 7) = b[7];
        } else {
          *((Byte*)lp) = b[7];
          *((Byte*)lp + 1) = b[6];
          *((Byte*)lp + 2) = b[5];
          *((Byte*)lp + 3) = b[4];
          *((Byte*)lp + 4) = b[3];
          *((Byte*)lp + 5) = b[2];
          *((Byte*)lp + 6) = b[1];
          *((Byte*)lp + 7) = b[0];
        }
        
        return l;
      }
      
      Double read_double(gzFile file) {
        Double d;
        Byte b[sizeof(d)];
        assert_error(file, gzread(file, b, sizeof(d)) == sizeof(d), "Buffer to short to read Double");
        Double *dp = &d;
        
        if (is_big_endian()) {
          *((Byte*)dp) = b[0];
          *((Byte*)dp + 1) = b[1];
          *((Byte*)dp + 2) = b[2];
          *((Byte*)dp + 3) = b[3];
          *((Byte*)dp + 4) = b[4];
          *((Byte*)dp + 5) = b[5];
          *((Byte*)dp + 6) = b[6];
          *((Byte*)dp + 7) = b[7];
        } else {
          *((Byte*)dp) = b[7];
          *((Byte*)dp + 1) = b[6];
          *((Byte*)dp + 2) = b[5];
          *((Byte*)dp + 3) = b[4];
          *((Byte*)dp + 4) = b[3];
          *((Byte*)dp + 5) = b[2];
          *((Byte*)dp + 6) = b[1];
          *((Byte*)dp + 7) = b[0];
        }
        
        return d;
      }
    public:
      begin_compound_t begin_compound;
      register_long_t register_long;
      register_short_t register_short;
      register_string_t register_string;
      register_float_t register_float;
      register_double_t register_double;
      register_int_t register_int;
      register_byte_t register_byte;
      register_byte_array_t register_byte_array;
      end_compound_t end_compound;
      
      begin_list_t begin_list;
      end_list_t end_list;
      error_handler_t error_handler;
      
      Parser() :
        context(NULL),
        begin_compound(default_begin_compound),
        register_long(NULL),
        register_short(NULL),
        register_string(NULL),
        register_float(NULL),
        register_double(NULL),
        register_int(NULL),
        register_byte(NULL),
        register_byte_array(NULL),
        end_compound(default_end_compound),
        begin_list(default_begin_list),
        end_list(default_end_list),
        error_handler(default_error_handler)
      {
      }
      
      Parser(void *context) :
        context(context),
        begin_compound(default_begin_compound),
        register_long(NULL),
        register_short(NULL),
        register_string(NULL),
        register_float(NULL),
        register_double(NULL),
        register_int(NULL),
        register_byte(NULL),
        register_byte_array(NULL),
        end_compound(default_end_compound),
        begin_list(default_begin_list),
        end_list(default_end_list),
        error_handler(default_error_handler)
      {
        this->context = context;
      }
      
      Byte read_tagType(gzFile file) {
        Byte type = read_byte(file);
        assert_error(file, type >= 0 && type <= TAG_Compound, "Not a valid tag type");
        return type;
      }
      
      void handle_list(String name, gzFile file) {
        Byte type = read_byte(file);
        Int length = read_int(file);
        
        begin_list(context, name, type, length);
        
        for (int i = 0; i < length; i++) {
          handle_type(type, name, file);
        }

        end_list(context);
      }

      void flush_byte_array(gzFile file) {
        Int length = read_int(file);
        assert_error(file, gzseek(file, length, SEEK_CUR) != -1,
          "Buffer to short to flush ByteArray");
      }
      
      void handle_byte_array(String name, gzFile file) {
        Int length = read_int(file);
        Byte *values = new Byte[length];
        assert_error(file, gzread(file, values, length) == length, "Buffer to short to read ByteArray");
        ByteArray *array = new ByteArray();
        array->values = values;
        array->length = length;
        register_byte_array(context, name, array);
      }

      void handle_compound(String name, gzFile file) {
        begin_compound(context, name);
        
        do {
          Byte type = read_tagType(file);
          
          if (type == TAG_End) {
            break;
          }
          
          name = read_string(file);
          handle_type(type, name, file);
        } while(1);
        
        end_compound(context);
      }
      
      void handle_type(Byte type, String name, gzFile file);
      void parse_file(const char *path);
  };
}

#endif /* _NBT_H_ */
