#include <iostream>
#include <string>
#include "./string_calculator.h"

using std::cout, std::endl;
using std::string;

unsigned int digit_to_decimal(char digit) {
	if (digit - 48 >= 10 || digit - 48 < 0){
		throw std::invalid_argument("Not a number");
	}
    int decimal = digit - 48;
    return decimal;
}

char decimal_to_digit(unsigned int decimal) {
    char digit = '0' + decimal;
	if (digit - 48 >= 10 || digit - 48 < 0) {
		throw std::invalid_argument("Not a decimal");
	}
    return digit;
}

string trim_leading_zeros(string num) {
	int j = num.size();
    if(num.at(0) == '-') {
        if(num.at(1)== '0'){
            while(num.at(1) == '0' && j > 2) {
                for(unsigned i = 1; i < num.size()-1; i++) {
					num.at(i)= num.at(i+1);
					}
                j = j - 1;
				if (j == 2 && num.at(1) == '0') {
					return "0";
				}
            }
        }
    }
    else if(num.at(0)=='0') {
       while(num.at(0) == '0' && j > 1) {
            for(unsigned i = 0; i < num.size()-1; i++) {
               num.at(i)= num.at(i+1);
            }
            j = j - 1;
      }
    }
	return num.substr(0,j);
}

string add(string lhs, string rhs) {
	if (lhs == "0"){
		return rhs;
	}
	else if (rhs == "0"){
		return lhs;
	}
	int negativeR = 0;
	int negativeL = 0;
	int answerNegative = 2;
	if (lhs.at(0) == '-'){
		negativeL = 1;
		lhs.replace(0,1,"0");
	}
	if (rhs.at(0) == '-'){
		negativeR = 1;
		rhs.replace(0,1,"0");
	}
	if (lhs.length() > rhs.length()){
		for (unsigned i = rhs.length(); i < lhs.length(); i++){
			rhs.insert(0,1,'0');
		}
	}
	if (rhs.length() > lhs.length()){
		for (unsigned i = lhs.length(); i < rhs.length(); i++){
			lhs.insert(0,1,'0');
		}
	}
	if (negativeL == 1 && negativeR == 1) {
		answerNegative = 1;
	}
	else if (negativeL == 0 && negativeR == 0) { 
		answerNegative = 0;
	}
	else { 
		size_t i = 0;
		while (answerNegative == 2 && i < lhs.length()) {
			int l;
			int r;
			l = digit_to_decimal(lhs.at(i));
			r = digit_to_decimal(rhs.at(i));
			if (l > r && negativeL == 1 && (l != 0 || r != 0)) {
				answerNegative = 1;
			}
			else if (l < r && negativeL == 1 && (l != 0 || r != 0)) {
				answerNegative = 0;
			}
			else if (r > l && negativeR == 1 && (l != 0 || r != 0)) {
				answerNegative = 1;
			}
			else if (r < l && negativeR == 1 && (l != 0 || r != 0)) {
				answerNegative = 0;
			}
			i++;
		}
		if (answerNegative == 2) {
			answerNegative = 0;
		}
	}
	int l;
	int r;
	int total = 0;
	int carryOver = 0;
	string zhs;
	for (int i = lhs.length() - 1; i >= 0; i--){
		l = digit_to_decimal(lhs.at(i));
		r = digit_to_decimal(rhs.at(i));
		if (answerNegative == 1) {
			if (negativeR == 1 && negativeL == 0) {
				l = l * -1;
				r = r * -1;
			}			
			else if (negativeL == 1 && negativeR == 0) {
				l = l * -1;
				r = r * -1;
			}
		}
		if (negativeR == 1) {
			r = r * -1;
		}
		if (negativeL == 1) {
			l = l * -1;
		}
		total = l + r + carryOver;
		if (total >= 10) {
			total = total - 10;
			carryOver = 1;
		}
		else if (total > 0) {
			carryOver = 0;
		}
		else if (total <= -10) {
			total = total * -1 - 10;
			carryOver = -1;
		}
		else if (total < 0) {
			if (negativeL == 1 && negativeR == 1) {
				total = total * -1;
				carryOver = 0;
			}
			else {
				total = 10 + l + r + carryOver;
				carryOver = -1;
			}
		}
		else if (total == 0) {
			carryOver = 0;
		}
		zhs.insert(0,1,decimal_to_digit(total));
	}
	if (carryOver != 0) {
		if (carryOver == -1) {
			carryOver = 1;
		}
		zhs.insert(0,1,decimal_to_digit(carryOver));
	}
	if (answerNegative == 1) {
		zhs.insert(0,1,'-');
	}
	return trim_leading_zeros(zhs);
	
}

string subtract(string lhs, string rhs) {
	if (rhs == "0") {
		return lhs;
	}
	int negativeR = 0;
	int negativeL = 0;
	int answerNegative = 2;
	if (lhs.at(0) == '-'){
		negativeL = 1;
		lhs.replace(0,1,"0");
	}
	if (rhs.at(0) == '-'){
		negativeR = 1;
		rhs.replace(0,1,"0");
	}
	if (lhs == "0") {
		if (negativeR == 1) {
			return trim_leading_zeros(rhs);
		}
		else {
			trim_leading_zeros(rhs);
			rhs.insert(0,1,'-');
			return rhs;
		}
	}
	if (lhs.length() > rhs.length()){
		for (unsigned i = rhs.length(); i < lhs.length(); i++){
			rhs.insert(0,1,'0');
		}
	}
	if (rhs.length() > lhs.length()){
		for (unsigned i = lhs.length(); i < rhs.length(); i++){
			lhs.insert(0,1,'0');
		}
	}
	if (negativeL == 0 && negativeR == 1) {
		answerNegative = 0;
	}
	else if (negativeL == 1 && negativeR == 0) { 
		answerNegative = 1;
	}
	else { 
		size_t i = 0;
		while (answerNegative == 2 && i < lhs.length()) {
			int l;
			int r;
			l = digit_to_decimal(lhs.at(i));
			r = digit_to_decimal(rhs.at(i));
			if (l > r && negativeL == 0 && negativeR == 0 && (l != 0 || r != 0)) {
				answerNegative = 0;
			}
			else if (l < r && negativeR == 0 && negativeL == 0 && (l != 0 || r != 0)) {
				answerNegative = 1;
			}
			else if (r > l && negativeR == 1 && negativeL == 1 && (l != 0 || r != 0)) {
				answerNegative = 0;
			}
			else if (r < l && negativeL == 1 && negativeR == 1 && (l != 0 || r != 0)) {
				answerNegative = 1;
			}
			i++;
		}
		if (answerNegative == 2) {
			answerNegative = 0;
		}
	}
	int l;
	int r;
	int total = 0;
	int carryOver = 0;
	string zhs;
	for (int i = lhs.length() - 1; i >= 0; i--) {
		l = digit_to_decimal(lhs.at(i));
		r = digit_to_decimal(rhs.at(i));
		if (answerNegative == 1) {
			if (negativeR == 0 && negativeL == 0) {
				l = l * -1;
				r = r * -1;
			}			
			else if (negativeL == 1 && negativeR == 1) {
				l = l * -1;
				r = r * -1;
			}
		}
		if (negativeR == 1) {
			r = r * -1;
		}
		if (negativeL == 1) {
			l = l * -1;
		}
		r = r * -1;
		total = l + r + carryOver;
		if (total >= 10) {
			total = total - 10;
			carryOver = 1;
		}
		else if (total > 0) {
			carryOver = 0;
		}
		else if (total <= -10) {
			total = total * -1 - 10;
			carryOver = -1;
		}
		else if (total < 0) {
			if (negativeL == 1 && negativeR == 0) {
				total = total * -1;
				carryOver = 0;
			}
			else {
				total = 10 + l + r + carryOver;
				carryOver = -1;
			}
		}
		else if (total == 0) {
			carryOver = 0;
		}
		zhs.insert(0,1,decimal_to_digit(total));
	}
	if (carryOver != 0) {
		if (carryOver == -1) {
			carryOver = 1;
		}
		zhs.insert(0,1,decimal_to_digit(carryOver));
	}
	if (answerNegative == 1) {
		zhs.insert(0,1,'-');
	}
	return trim_leading_zeros(zhs);
}

string multiply(string lhs, string rhs) {
	if (lhs == "0"){
		return "0";
	}
	else if (rhs == "0"){
		return "0";
	}
	int negativeR = 0;
	int negativeL = 0;
	int answerNegative = 0;
	if (lhs.at(0) == '-'){
		negativeL = 1;
		lhs.replace(0,1,"0");
	}
	if (rhs.at(0) == '-'){
		negativeR = 1;
		rhs.replace(0,1,"0");
	}
	if ((negativeL == 0 && negativeR == 0) || (negativeL == 1 && negativeR == 1)) {
		answerNegative = 0;
	}
	else {
		answerNegative = 1;
	}
	string zhs = "0";
	string addR;
	int z = 0;
	int carryOver = 0;
	string addOn;
	for (int i = rhs.length()-1; i >= 0; i--) {
		for (int j = lhs.length() -1; j >= 0; j--) {
			z = digit_to_decimal(lhs.at(j)) * digit_to_decimal(rhs.at(i))+carryOver;
			carryOver = z/10;
			z = z%10;
			addOn.insert(0,1,decimal_to_digit(z));
		}
		if (carryOver != 0) {
			addOn.insert(0,1,decimal_to_digit(carryOver));
			carryOver = carryOver/10;
		}
		zhs = add(zhs,addOn);
		addOn = "";
		addOn.insert(0,rhs.length()-i,'0');
	}
	if (answerNegative == 1) {
		zhs.insert(0,1,'-');
	}
	
    return trim_leading_zeros(zhs);
}