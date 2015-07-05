#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "connector.h"

using namespace std;

struct Piece{
	char type;
	char owner;
};

struct Map{
	Piece *none;
	string line[10];
	Piece *local_map[10][10];
	int last_source_x;
	int last_source_y;
	int last_dest_x;
	int last_dest_y;
};

class Move{
public:
	char piece;
	char dest;
	int fx;
	int fy;
	int tx;
	int ty;
	Move(char p, char d, int f1, int f2, int t1, int t2){
		piece = p; dest = d; fx = f1; fy = f2; tx = t1; ty = t2; 
	}
	void print(){
		cout << "piece:" <<piece << " dest:" << dest 
			<< "from:" << fx << " " << fy << " to:" << tx 
			<< " " << ty << endl;
	}
};
struct Moves{
	vector<Move> moves;
};


int start(){
	int con = connect("localhost",3720);
	sendln(con,"RED");
	if(receiveln(con)=="INVALID COLOR") exit(0);
	sendln(con,"M653262226 4288924BB5 727165B452 73334BFBB3");
	if(receiveln(con)!="OK") exit(0);
	return con;
}

void print_moves(Moves &mov){
	for(Move i : mov.moves){
		i.print();
	}
	cout << "results!!!!" << endl;
}

void check_border(Map map, Moves &mov, int x, int y){
	int j = 0;
	for(int i = -1; i <= 1; i+=2){
		if(0 <= i+y && i+y <= 9 && 0 <= j+x && j+x <= 9){
			char temp = map.line[i+y].at(j+x);
			if(temp == '.'||temp == '#'||temp == '$'){
				mov.moves.push_back(Move(map.line[y].at(x), temp, x, y, j+x, i+y));
			}
		}
	}
	int i = 0;
	for(int j = -1; j <= 1; j+=2){
		if(0 <= i+y && i+y <= 9 && 0 <= j+x && j+x <= 9){
			char temp = map.line[i+y].at(j+x);
			if(temp == '.'||temp == '#'||temp == '$'){
				mov.moves.push_back(Move(map.line[y].at(x), temp, x, y, j+x, i+y));
			}
		}
	}
}

void check_line(Map map, Moves &mov, const int x, const int y){
	for(int i = x + 1; i < 10; i++){
		char temp = map.line[y].at(i);
		if(temp == '.' || temp == '#'){
			assert((i<10) && (i>= 0));
			mov.moves.push_back(Move(map.line[y].at(x), temp, x, y, i, y));
			if(temp != '.') break;
		} else break;
	
	}
	for(int i = x - 1; i >= 0; i--){
		if(i >= -9){
			char temp = map.line[y].at(i);
			if(temp == '.' || temp == '#'){
				assert(i >= 0);
				assert(i < 10);
				mov.moves.push_back(Move(map.line[y].at(x), temp, x, y, i, y));
				if(temp != '.') break;
			} else break;
		}else break;
	}
	for(int i = y + 1; i < 10; i++){
		if(i <= 9){
			char temp = map.line[i].at(x);
			if(temp == '.' || temp == '#'){
				mov.moves.push_back(Move(map.line[y].at(x), temp, x, y, x, i));
				if(temp != '.') break;
			} else break;
		}else break;
	}
	for(int i = y - 1; i >= 0; i--){
		if(i >= -9){
			char temp = map.line[i].at(x);
			if(temp == '.' || temp == '#'){
				mov.moves.push_back(Move(map.line[y].at(x), temp, x, y, x, i));
				if(temp != '.') break;
			} else break;
		}else break;
	}
}
Moves get_valid_moves(Map map){
	char loc;
	Moves mov;
	for(int i = 0; i < 10; i++){
		for(int j = 0; j < 10; j++){
			loc = map.line[i].at(j);
			if(loc == 'F'||loc == 'B'||loc == '~'||loc == '.'||loc == '#' || loc == '2');
			else{
				check_border(map, mov, j, i);
			}
			if(loc == '2'){
				check_line(map, mov, j, i);
			}
		}
	}
	//print_moves(mov);
	return mov;
}

Map get_map(int con, Map map){
	for(int i = 0; i < 10; i++){
		map.line[i] = receiveln(con);
		//cout << map.line[i] << endl;
	} 
	return map;
}


char numb_to_char(int numb){
	return 'A' + numb;
}

void send_move(int con,Map &map, Move mov){
	string str = "MOVE "; 
	str.push_back(numb_to_char(mov.fx));
	str += to_string(10 - mov.fy);
	str += " ";
	str.push_back(numb_to_char(mov.tx)); 
	str += to_string(10 - mov.ty);
	map.last_source_x = mov.fx;
	map.last_source_y = mov.fy;
	map.last_dest_x = mov.tx;
	map.last_dest_y = mov.ty;
	sendln(con, str);
}