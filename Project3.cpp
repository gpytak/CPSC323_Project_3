// ============================================================================
// Programmer: Vincent Hsu, Gregory Pytak, Ethan Ton
// Date: 05/13/2022
// Class: CPSC 323
// Project: Project 3
// ============================================================================
#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <stack>

using namespace std;

bool isComment = false;
bool PUSH = false;
int inputCount = 0;
string input, top, printRules[60];
ofstream oFile;

enum TransitionStates {
	// REJECT is the starting state and final state.
	REJECT = 0,
	INTEGER = 1,
	REAL = 2,
	OPERATOR = 3,
	STRING = 4,
	SPACE = 5,
	SEPARATOR = 6,
	COMMENT = 7,
	UNKNOWN = 8
};

// ============================================================================
//  STRUCT Token
// ============================================================================
struct Tokens
{
	string lexemeValue;
	int lexeme;
	string tokenName;
};

// ============================================================================
//  Function Prototypes
// ============================================================================
vector<Tokens> lexer(string fileInput);
int getCol(char character);
string tokenName(string token, int lexeme);

// ============================================================================
//  Integer Table
// ============================================================================
/* INTEGER, REAL, OPERATOR, STRING, SPACE, SEPARATOR, COMMENT, UNKNOWN  */
int table[9][9] =
{ {REJECT,   INTEGER,       REAL,          OPERATOR,      STRING,       SPACE,         SEPARATOR,    COMMENT,  UNKNOWN},
/* STATE 1 */ {INTEGER,   INTEGER    ,   REAL       ,   REJECT     ,   STRING     ,  REJECT     ,   REJECT     ,  COMMENT,  REJECT  },
/* STATE 2 */ {REAL,      REAL       ,   UNKNOWN    ,   REJECT     ,   REJECT     ,  REJECT     ,   REJECT     ,  COMMENT,  REJECT  },
/* STATE 3 */ {OPERATOR,  REJECT     ,   REJECT     ,   REJECT     ,   REJECT     ,  REJECT     ,   REJECT     ,  COMMENT,  REJECT  },
/* STATE 4 */ {STRING,    STRING     ,   REJECT     ,   REJECT     ,   STRING     ,  REJECT     ,   REJECT     ,  COMMENT,  REJECT  },
/* STATE 5 */ {SPACE,     REJECT     ,   REJECT     ,   REJECT     ,   REJECT     ,  REJECT     ,   REJECT     ,  COMMENT,  REJECT  },
/* STATE 6 */ {SEPARATOR, REJECT     ,   REJECT     ,   REJECT     ,   REJECT     ,  REJECT     ,   REJECT     ,  COMMENT,  REJECT  },
/* STATE 7 */ {COMMENT,  COMMENT     ,   COMMENT    ,   COMMENT    ,   COMMENT    ,  COMMENT    ,   COMMENT    ,  REJECT ,  COMMENT  },
/* STATE 8 */ {UNKNOWN,   UNKNOWN    ,   UNKNOWN    ,   UNKNOWN    ,   UNKNOWN    ,  UNKNOWN    ,   UNKNOWN    ,  UNKNOWN,  UNKNOWN  } };


/*ParseTable*/
//S->A
//A->id=E
//E->TQ
//Q->+TQ|-TQ|epsilon
//T->FR
//R->*FR|/FR|epsilon
//F->id|(E)
const int parseTableRowCount = 7;
const int parseTableColumnCount = 11;
string parseTable[parseTableRowCount][parseTableColumnCount] = {
	{"0", "$"	, "id"	, "="	, "+"	, "-"	, "*"	, "/"	, "("	, ")"	, ";"},
	{"A", "0"	, "id=E", "id=E", "0"	, "0"	, "0"	, "0"	, "E"	, "0"	, "0"},
	{"E", "0"	, "T Q"	, "0"	, "0"	, "0"	, "0"	, "0"	, "T Q"	, "0"	, "0"},
	{"Q", "eps"	, "0"	, "0"	, "+T Q"	, "-T Q"	, "0"	, "0"	, "0"	, "eps"	, "0"},
	{"T", "0"	, "F R"	, "0"	, "0"	, "0"	, "0"	, "0"	, "F R"	, "0"	, "0"},
	{"R", "eps"	, "0"	, "0"	, "0"	, "0"	, "*F R"	, "/F R"	, "0"	, "eps"	, "0"},
	{"F", "0"	, "id"	, "0"	, "0"	, "0"	, "0"	, "0"	, "(E)"	, "0"	, "0"},
};

int main()
{
	ifstream inFile;
	int input;
	string currInput = "";
	string fileInput = "";
	vector<Tokens> tokens;
	vector<Tokens> idTerm;
	vector<Tokens> tableEntry;
	vector<string> fileInputTP;
	stack<string> parseStack;
	parseStack.push("$");
	parseStack.push("S");

	cout << "Please choose which input file to analyze:" << endl;
	cout << "1: Input File 1" << endl;
	cout << "2: Input File 2" << endl;
	cout << "3: Input File 3" << endl;

	// Receive input from user to select which file to open
	// Loops if input received is not acceptable
	while (true)
	{
		cin >> input;
		if (input == 1 || input == 2 || input == 3)
		{
			break;
		}
		cout << "Please input 1, 2, or 3." << endl;
	}

	// Uses user input to select which file to open
	if (input == 1)
	{
		inFile.open("Input1.txt");
		oFile.open("output1.txt");
	}
	else if (input == 2)
	{
		inFile.open("Input2.txt");
		oFile.open("output2.txt");
	}
	else
	{
		inFile.open("Input3.txt");
		oFile.open("output3.txt");
	}
	// Check if both out file and in file are properly opened
	if (!inFile.is_open() || !oFile.is_open())
	{
		cout << "Failed to open file." << endl;
		return 0;
	}

	while (getline(inFile, fileInput))
	{
		tokens = lexer(fileInput);

		// Setting fileInputTP with fileInput and $
		for (int i = 0; i < tokens.size(); i++)
		{
			fileInputTP.push_back(tokens[i].lexemeValue);
		}
		fileInputTP.push_back("$");

		// Parser
		while (parseStack.top() != "$")
		{
			// top = Top Production Symbol | currInput = Top Input String | idTerm = currInput Token Name
			PUSH = false;
			top = parseStack.top();
			currInput = fileInputTP[inputCount];
			idTerm = lexer(currInput);
			if (idTerm[0].tokenName == "IDENTIFIER")
			{
				currInput = "id";
			}

			cout << "TOP: " << top << endl;
			cout << "INPUT: " << currInput << endl;
			//cout << "idTerm: " << idTerm[0].tokenName << endl;

			if (top == "$" || top == "id" || top == "=" || top == "+" || top == "-" || top == "*" || top == "/" || top == "(" || top == ")" || top == ";")
			{
				if (top == currInput)
				{
					//oFile << "Token: " << tokens[inputCount].tokenName << " \t" << "Lexeme: " << tokens[inputCount].lexemeValue;
					//oFile << ruleOutputForTable(tokens[inputCount].lexemeValue) << endl;

					parseStack.pop();
					inputCount++;
				}
				else
				{
					cout << "Error, terminal not found" << endl;
				}
			}
			else
			{
				// remove starting symbol
				if (top == "S")
				{
					parseStack.pop();
					parseStack.push("A");
				}
				else
				{
					//non-terminal (row) match
					for (int r = 0; r < parseTableRowCount; r++)
					{
						if (parseTable[r][0] == top)
						{
							//input terminal (column) match
							for (int c = 0; c < parseTableColumnCount; c++)
							{
								if (parseTable[r][c] != "0")
								{
									tableEntry = lexer(parseTable[0][c]);
									for (int i = 0; i < tableEntry.size(); i++)
									{
										//cout << "." << parseTable[r][c] << endl;
										if (tableEntry[i].lexemeValue == currInput)
										{
											//check if valid table entry
											if (parseTable[r][c] != "0")
											{
												//  tableEntry = Table Lexeme Value 
												tableEntry = lexer(parseTable[r][c]);
												parseStack.pop();
												for (int k = (tableEntry.size() - 1); k >= 0; k--)
												{
													parseStack.push(tableEntry[k].lexemeValue);
												}
												PUSH = true;
												break;
											}
											else
											{
												cout << "Error, table spot is 0" << endl;
											}
										}
									}
								}
								if (!PUSH && c == (parseTableColumnCount - 1))
								{
									if (top == "E")
									{
										parseStack.pop();
										parseStack.push("Q");
										parseStack.push("T");
									}
									else if (top == "Q")
									{
										parseStack.pop();
									}
									else if (top == "T")
									{
										parseStack.pop();
										parseStack.push("R");
										parseStack.push("F");
									}
									else if (top == "R")
									{
										parseStack.pop();
									}
									break;
								}
							}
						}
					}
				}
			}

		}
	}
	oFile.close();
	inFile.close();
	return 0;
}

// ============================================================================
//  Vector lexer
// ============================================================================
vector<Tokens> lexer(string fileInput)
{
	Tokens type;
	vector<Tokens> tokens;
	string currentToken = "";
	int col = REJECT;
	int currentState = REJECT;
	int previousState = REJECT;
	char currentChar = ' ';

	for (int i = 0; i < fileInput.length();)
	{
		currentChar = fileInput[i];
		col = getCol(currentChar);
		currentState = table[currentState][col];

		if (isComment == false && col == COMMENT)
		{
			isComment = true;
		}
		else if (isComment == true && col == COMMENT)
		{
			isComment = false;
		}
		else if (isComment == true && col != COMMENT)
		{
			currentState = COMMENT;
		}

		if (currentState == REJECT)
		{
			if (previousState != SPACE && previousState != COMMENT)
			{
				type.lexemeValue = currentToken;
				type.lexeme = previousState;
				type.tokenName = tokenName(type.lexemeValue, type.lexeme);
				tokens.push_back(type);
			}
			currentToken = "";
		}
		else
		{
			currentToken += currentChar;
			i++;
		}
		previousState = currentState;
	}
	if (currentState != SPACE && currentToken != "" && previousState != COMMENT)
	{
		type.lexemeValue = currentToken;
		type.lexeme = currentState;
		type.tokenName = tokenName(type.lexemeValue, type.lexeme);
		tokens.push_back(type);
	}
	return tokens;
}

// ============================================================================
//  int getCol
//      INPUT  - char
//      OUTPUT - Column number associated with the current input character
// ============================================================================
int getCol(char character)
{
	if (character == '!')
	{
		return COMMENT;
	}
	if (isspace(character))
	{
		return SPACE;
	}
	else if (isdigit(character))
	{
		return INTEGER;
	}
	else if (character == '.')
	{
		return REAL;
	}
	else if (isalpha(character) || character == '$')
	{
		return STRING;
	}
	else if (character == '+' || character == '-' || character == '/' || character == '*' || character == '==' || character == '=' ||
		character == '>' || character == '<' || character == '%')
	{
		return OPERATOR;
	}
	else if (character == '(' || character == ')' || character == '{' || character == '}' || character == ',' || character == ';' || character == ':')
	{
		return SEPARATOR;
	}
	else if (ispunct(character))
	{
		return SEPARATOR;
	}
	return UNKNOWN;
}

// ============================================================================
//  tokenName
//      INPUT  - currentState
//      OUTPUT - Name of Lexeme
// ============================================================================
string tokenName(string token, int lexeme)
{
	switch (lexeme)
	{
	case INTEGER:
		return "INTEGER";
		break;
	case REAL:
		return "REAL  ";
		break;
	case OPERATOR:
		return "OPERATOR";
		break;
	case SEPARATOR:
		return "SEPARATOR";
		break;
	case STRING:
		if (token == "int" || token == "float" || token == "bool" || token == "if" || token == "else" || token == "then" ||
			token == "do" || token == "while" || token == "for" || token == "and" || token == "or" || token == "function")
		{
			return "KEYWORD";
		}
		else
		{
			return "IDENTIFIER";
		}
		break;
	case COMMENT:
		return "COMMENT";
		break;
	case UNKNOWN:
		return "UNKNOWN";
		break;
	case SPACE:
		return "SPACE";
		break;
	default:
		return "ERROR";
		break;
	}
}// =	===========================================================================
