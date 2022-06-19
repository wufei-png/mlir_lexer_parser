//===- Lexer.h - Lexer for the Tiny language -------------------------------===//
//===----------------------------------------------------------------------===//
//
// This file implements a simple Lexer for the Tiny language.
//
//===----------------------------------------------------------------------===//

#ifndef TINY_LEXER_H
#define TINY_LEXER_H

#include "llvm/ADT/StringRef.h"

#include <memory>
#include <string>

namespace tiny {

/// Structure definition a location in a file.
struct Location {
  std::shared_ptr<std::string> file; ///< filename.
  int line;                          ///< line number.
  int col;                           ///< column number.
};

// List of Token returned by the lexer.
enum Token : int {
  tok_semicolon = ';',
  tok_parenthese_open = '(',
  tok_parenthese_close = ')',
  tok_bracket_open = '{',
  tok_bracket_close = '}',
  tok_sbracket_open = '[',
  tok_sbracket_close = ']',

  tok_eof = -1,

  // commands
  tok_return = -2,
  tok_var = -3,
  tok_def = -4,
  tok_struct = -5,

  // primary
  tok_identifier = -6,

  //c语言中标识符是由字母（A-Z,a-z）、数字（0-9）、下划线“_”组成，并且首字符不能是数字，但可以是字母或者下划线。
  tok_number = -7,
  // tok_invalid_identifier = -8// basic part ;
};

/// The Lexer is an abstract base class providing all the facilities that the
/// Parser expects. It goes through the stream one token at a time and keeps
/// track of the location in the file for debugging purpose.
/// It relies on a subclass to provide a `readNextLine()` method. The subclass
/// can proceed by reading the next line from the standard input or from a
/// memory mapped file.
class Lexer {
public:
  /// Create a lexer for the given filename. The filename is kept only for
  /// debugging purpose (attaching a location to a Token).
  Lexer(std::string filename)
      : lastLocation(
            {std::make_shared<std::string>(std::move(filename)), 0, 0}) {}
  virtual ~Lexer() = default;

  /// Look at the current token in the stream.
  Token getCurToken() { return curTok; }

  /// Move to the next token in the stream and return it.
  Token getNextToken() { return curTok = getTok(); }

  /// Move to the next token in the stream, asserting on the current token
  /// matching the expectation.
  void consume(Token tok) {
    assert(tok == curTok && "consume Token mismatch expectation");
    getNextToken();
  }

  /// Return the current identifier (prereq: getCurToken() == tok_identifier)
  llvm::StringRef getId() {
    assert(curTok == tok_identifier);
    return identifierStr;
  }

  /// Return the current number (prereq: getCurToken() == tok_number)
  double getValue() {
    assert(curTok == tok_number);
    return numVal;
  }

  /// Return the location for the beginning of the current token.
  Location getLastLocation() { return lastLocation; }

  // Return the current line in the file.
  int getLine() { return curLineNum; }

  // Return the current column in the file.
  int getCol() { return curCol; }

private:
  /// Delegate to a derived class fetching the next line. Returns an empty
  /// string to signal end of file (EOF). Lines are expected to always finish
  /// with "\n"
  virtual llvm::StringRef readNextLine() = 0;

  /// Return the next character from the stream. This manages the buffer for the
  /// current line and request the next line buffer to the derived class as
  /// needed.
  int getNextChar() {
    // The current line buffer should not be empty unless it is the end of file.
    if (curLineBuffer.empty())
      return EOF;
    ++curCol;
    auto nextchar = curLineBuffer.front();
    curLineBuffer = curLineBuffer.drop_front();
    if (curLineBuffer.empty())
      curLineBuffer = readNextLine();
    if (nextchar == '\n') {
      ++curLineNum;
      curCol = 0;
    }
    return nextchar;
  }

  ///  Return the next token from standard input.
  Token getTok() {
    // Skip any whitespace.
    while (isspace(lastChar))
      lastChar = Token(getNextChar());

    // Save the current location before reading the token characters.
    lastLocation.line = curLineNum;
    lastLocation.col = curCol;

    //TODO: Here you need to identify:
    //      1. Keywords: "return", "def", "var"
    //      2. Variable names
    //      Note that a variable name should follow some rules:
    //      1) It should start with a letter.
    //      2) It consists of letters, numbers and underscores.
    //      3) If there are numbers in the name, they should be at the end of the name. 
    //      For example, these names are valid: a123, b_4, placeholder
  
    //Hints: 1. You can refer to the implementaion of identifying a "number" in the same function.
    //       2. Some functions that might be useful:  isalpha(); isalnum();
    //       3. Maybe it is better for you to go through the whole lexer before you get started.

    /* 
     *
     *  Write your code here.
     *
     */
    // 要求：变量名以字母开头 变量名由字母、数字和下划线组成别的不行
    if (isalpha(lastChar))//isalpha：判断一个Token开头是否是字母,如果是的话，那它就是变量或标识符，if语句内的工作是识别关键字和判断标识符（判断是否合法）如果不是字母，进行之后的判断 是数字啥的
    {
    	identifierStr = "";
    	identifierStr += (char)lastChar;
    	bool lastDigit = false;
    	//bool invalid = false;
    	lastChar = Token(getNextChar());
    	while (isalnum((lastChar)) || lastChar == '_')//判断一个字符是否是字母，数字下划线，如果不是，跳出while循环，得到一个token,否则一直读入直到停止while循环
    	{
    		if(isdigit(lastChar))
    		{
    			lastDigit = true;
    		}
    		else{
    			if(lastDigit)//如果之前已经出现过数字但是现在这个char不是数字，那么就是非法的标识符
    			{
    				return Token(lastChar);//把这个非法的char弄成Token，返回给Parser
    			} 
    		}
    		identifierStr += (char)lastChar;
    		lastChar = Token(getNextChar());
    	}
    	
    	if (identifierStr == "return")//判断是否是关键字
    	{
    		return tok_return;
    	}
    	else if (identifierStr == "def")
    	{
    		return tok_def;
    	}
    	else if (identifierStr == "var")
    	{
    		return tok_var;
    	}
    	
    	// else if (invalid)
    	// {
    	// 	//return tok_invalid_identifier;//没有这个定义？
      //   return Token(lastChar);
    	// }
    	else
    	{
    		return tok_identifier;//合法标识符
    	}
    }

    // Identify a number: [0-9] ([0-9.])*
    if (isdigit(lastChar)) {
      std::string numStr;
      do {
        numStr += lastChar;
        lastChar = Token(getNextChar());
      } while (isdigit(lastChar) || lastChar == '.');//数字或者小数点就一直加

      numVal = strtod(numStr.c_str(), nullptr);
      //c_str()：生成一个const char*指针，指向以空字符终止的数组。
      //C 库函数 double strtod(const char *str, char **endptr) 把参数 str 所指向的字符串转换为一个浮点数（类型为 double 型）
      return tok_number;
    }

    if (lastChar == '#') {
      // Comment until end of line.
      do {
        lastChar = Token(getNextChar());
      } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

      if (lastChar != EOF)
        return getTok();
    }

    // Check for end of file.  Don't eat the EOF.
    if (lastChar == EOF)
      return tok_eof;

    // Otherwise, just return the character as its ascii value.
    Token thisChar = Token(lastChar);
    lastChar = Token(getNextChar());
    return thisChar;
  }

  /// The last token read from the input.
  Token curTok = tok_eof;

  /// Location for `curTok`.
  Location lastLocation;

  /// If the current Token is an identifier, this string contains the value.
  std::string identifierStr;

  /// If the current Token is a number, this contains the value.
  double numVal = 0;

  /// The last value returned by getNextChar(). We need to keep it around as we
  /// always need to read ahead one character to decide when to end a token and
  /// we can't put it back in the stream after reading from it.
  Token lastChar = Token(' ');

  /// Keep track of the current line number in the input stream
  int curLineNum = 0;

  /// Keep track of the current column number in the input stream
  int curCol = 0;

  /// Buffer supplied by the derived class on calls to `readNextLine()`
  llvm::StringRef curLineBuffer = "\n";
};

/// A lexer implementation operating on a buffer in memory.
class LexerBuffer final : public Lexer {
public:
  LexerBuffer(const char *begin, const char *end, std::string filename)
      : Lexer(std::move(filename)), current(begin), end(end) {}

private:
  /// Provide one line at a time to the Lexer, return an empty string when
  /// reaching the end of the buffer.
  llvm::StringRef readNextLine() override {
    auto *begin = current;
    while (current <= end && *current && *current != '\n')
      ++current;
    if (current <= end && *current)
      ++current;
    llvm::StringRef result{begin, static_cast<size_t>(current - begin)};
    return result;
  }
  const char *current, *end;
};
} // namespace tiny

#endif // TINY_LEXER_H
