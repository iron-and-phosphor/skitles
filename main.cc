#include <iostream>
#include <vector>
#include <string>

#include "connector.h"
#include "fase1.cpp"

#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */

using namespace std;

string rules[256];

void set_rules(){
	rules[int('M')] = "B1#";
	rules[int('9')] = "BM#";
	rules[int('8')] = "BM9";
	rules[int('7')] = "BM98";
	rules[int('6')] = "BM9876";
	rules[int('5')] = "BM98765";
	rules[int('4')] = "BM987654";
	rules[int('3')] = "M9876543";
	rules[int('2')] = "BM98765432";
	rules[int('1')] = "B987654321#$";
	
}

Moves filter_moves(Moves &mov,Map &map){
	Moves rule_filter;
	set_rules();
	for(Move m:mov.moves){
		bool test = true;
		for(char rule:rules[int(m.piece)]){
			if(map.local_map[m.tx][m.ty]->type == rule){test = false;}
		}
		if(test)rule_filter.moves.push_back(m);
	}
	return rule_filter;
}

Move choose_move(Moves &mov, Map &map,int random){
	Moves attack;
	for(Move m:mov.moves){
		if(m.dest != '.'){
			attack.moves.push_back(m);
		}
	} 
	if(attack.moves.size() > 0){ 
		Move highest = attack.moves[0];
		for(Move m:attack.moves){
			if(highest.dest < m.dest) highest = m;
		}
		return highest;
	}
	else {
		if(mov.moves.size()==0){
			Moves test = get_valid_moves(map);
			print_moves(test);
		}
		assert(mov.moves.size()!=0);
		random = rand() % mov.moves.size();
		return mov.moves[random];
	}
}


void print_local_map(Map &map){
	string h;
	string t;
	for(int y = 0; y < 10; y++){
		for(int x = 0; x < 10; x++){
			if(map.local_map[x][y]->owner == 'B') h = "\x1B[34m"; 
			else if(map.local_map[x][y]->owner == 'R')h = "\x1B[31m";
			else if(map.local_map[x][y]->owner == 'G')h = "\x1B[32m";
			else if(map.local_map[x][y]->owner == 'W')h = "\x1B[36m";
			else h = "";
			cout << h << map.local_map[x][y]->type << "\x1B[0m";
		}
		cout << endl;
	}
	cout << "--------------" << endl;
	
}

void build_local_map(Map &map){
	Piece *none = new Piece;
	none->type = '.';
	none->owner = 'G';
	map.none = none;
	for(int y = 0; y < 10; y++){
		for(int x = 0; x < 10; x++){
			Piece *temp = new Piece;
			temp->type = map.line[y].at(x);
			if(temp->type == '#') temp->owner = 'B';
			else if(temp->type!='~'&&temp->type!= '.'&&temp->type!='#') temp->owner = 'R';
			else if(temp->type=='~') temp->owner = 'W';
			else if(temp->type=='.') temp->owner = 'G';
			else temp->owner = 'W';
			map.local_map[x][y] = temp;
		}
	}
}
Move get_last_move(int con){
	char sx,dx;
	int  sy,dy;
	string input;
	sendln(con,"GETLASTMOVE");
	input = receiveln(con);
	if(input == "LASTMOVE NONE") return Move('?','?',0,0,0,0);
	sscanf(&input[0],"LASTMOVE %c%d %c%d",&sx,&sy,&dx,&dy);
	return Move('g','g',sx-'A',10-sy,dx-'A',10-dy);
}

void update_local_map(int con, Map &map){
	char sx,dx;
	int  sy,dy;
	string input;
	sendln(con,"GETLASTMOVE");
	input = receiveln(con);
	if(input == "LASTMOVE NONE") return;
	sscanf(&input[0],"LASTMOVE %c%d %c%d",&sx,&sy,&dx,&dy);
	map.local_map[dx-'A'][10-dy] = map.local_map[sx-'A'][10-sy];
	map.local_map[sx-'A'][10-sy] = map.none;
	if(map.local_map[dx-'A'][10-dy]->type == '#') map.local_map[dx-'A'][10-dy]->type = '$';
	if(abs(sx-dx) > 1 || abs(sy-dy) > 1) map.local_map[dx-'A'][10-dy]->type = '2';

	map.last_dest_x = dx-'A';
	map.last_dest_y = 10-dy;
}
void update_defend_map(int con, Map &map, string input){
	if(input == "DEFEND NONE") update_local_map(con,map); 
	else{
		Move move = get_last_move(con);
		char a,d,v;
		sscanf(&input[0],"DEFEND %c %c %c",&a,&d,&v);
		if(v=='0' && move.piece != '?'){
			map.local_map[move.fx][move.fy] = map.none;
			map.local_map[move.tx][move.ty] = map.none;
			return;
		}
		if(a==v){
			map.local_map[move.tx][move.ty] = map.local_map[move.fx][move.fy];
			map.local_map[move.fx][move.fy] = map.none;
			map.local_map[move.tx][move.ty]->type = v;
		}	
		else{
			map.local_map[move.fx][move.fy] = map.none;
			map.local_map[move.tx][move.ty]->type = v;
		}
	}
}
void update_attack_map(Map &map, string input){
	if(input == "ATTACK NONE"){
		map.local_map[map.last_dest_x][map.last_dest_y] = map.local_map[map.last_source_x][map.last_source_y];
		map.local_map[map.last_source_x][map.last_source_y] = map.none;
	}
	else{
		char a,d,v;
		sscanf(&input[0],"ATTACK %c %c %c",&a,&d,&v);
		if(v=='0'){
			map.local_map[map.last_source_x][map.last_source_y] = map.none;
			map.local_map[map.last_dest_x][map.last_dest_y] = map.none;
			return;
		}
		if(a==v){
			map.local_map[map.last_dest_x][map.last_dest_y] = map.local_map[map.last_source_x][map.last_source_y];
			map.local_map[map.last_source_x][map.last_source_y] = map.none;
			map.local_map[map.last_dest_x][map.last_dest_y]->type = v;
		}	
		else{
			map.local_map[map.last_source_x][map.last_source_y] = map.none;
			map.local_map[map.last_dest_x][map.last_dest_y]->type = v;
		}
	}
}


void react(int con){
	Map map;
	bool first_time = true;
	int chosen_move = 0;
	while(true){
		string downl = receiveln(con);
		if(downl.substr(0,6) == "DEFEND") {
			map = get_map(con,map);
			if(first_time){build_local_map(map); first_time = false;}
			update_defend_map(con,map,downl);
			print_local_map(map);
			Moves mov = get_valid_moves(map);
			Moves mov2 = filter_moves(mov,map);
			Move the_chosen_one = choose_move(mov2,map,chosen_move);
			chosen_move++;
			send_move(con,map,the_chosen_one);
		}
		else if(downl.substr(0,6) == "ATTACK") update_attack_map(map,downl);
		else if(downl == "INVALID MOVE") exit(0);
		else if(downl == "INVALID COMMAND") exit(0);
		else if(downl == "WIN") exit(0);
		else if(downl == "LOSE") exit(0);

	}
}

int main(int argc, char const *argv[]){
	string ip = "localhost";
	int port = 3720;
	if(argc > 1) ip = argv[1];
	if(argc > 2) port = atol(argv[2]); 
	int con = start(ip, port);
	srand (time(NULL));
	react(con);
	cout << "done!!!!!" << endl;
	string input;
	//cin >> input;
	return 0;
}