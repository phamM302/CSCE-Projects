#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "./string_calculator.h"

using std::cout, std::endl, std::cin;
using std::string;

int main() {
    cout << "String Calculator" << endl;
    cout << "\"q\" or \"quit\" or ctrl+d to exit" << endl << ">> ";
	string userString;
	std::getline(cin, userString);
	cout << endl;
	string zhs;
	while (userString != "q" && userString != "quit") {
		std::istringstream inSS (userString);
		string rhs = "0";
		string lhs = "0";
		if (lhs == "ans") {
			lhs = zhs;
		}
		if (rhs == "ans") {
			rhs = zhs;
		}
		string operation = "+";
		inSS >> lhs;
		inSS >> operation;
		inSS >> rhs;
		if (operation == "+") {
			zhs = add(lhs,rhs);
		}
		else if (operation == "-") {
			zhs = subtract(lhs,rhs);
		}
		else if (operation == "*") {
			zhs = multiply(lhs,rhs);
		}
		cout << "ans =" << endl << endl;
		cout << "    " << zhs << endl << endl;
		cout << ">> "; 
		std::getline(cin, userString);
		cout << endl;
	}
	cout << "farvel!" << endl << endl;
}