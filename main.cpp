#include <iostream>
#include <queue>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>
#include <gmpxx.h>

using std::queue;
using std::time;
using std::rand;
using std::srand;
using std::string;
using std::stringstream;

const char hex[] = "0123456789ABCDEF";
const char cip[] = "abcdefghijklmnopqrstuvwxyz.aeio.";

typedef unsigned short int word;

typedef struct
{
  unsigned int machine_no: 8;
  unsigned int data: 8;
} instruction;

instruction convert( word w )
{
  instruction i;
  i.machine_no = w%256;
  i.data = w >> 8;
}

word convert( instruction i )
{
  return (i.data << 8) + i.machine_no;
}

class machine
{
public:
  word value;
  machine() : value(0) {}

  word run( unsigned int data )
  {
    for(int i=0; i<3; ++i)
      {
	switch(data%8)
	  {
	
	  case 0: value = (1 + value%2 + value*2 + value%3 + value%17) xor value; break;
	  case 1: value -= 1; break;
	  case 2: value *= 3; break;
	  case 3: value /= 2; break;
	  case 4: value = value xor 0xFFFF; break;
	  case 5: value = value xor ((value*7)%257); break;
	  case 6: value = value*value; break;
	  case 7: value += 1; break;
	
	  }
	data/=8;
      }
    return value;
  }
  
 
};

void
print( const char *c )
{
  while( *c != 0 )
    addch(*(c++));
}

void
print(word n)
{
  addch( hex[n>>12] );
  addch( hex[(n>>8)%16] );
  addch( hex[(n>>4)%16] );
  addch( hex[n%16] );
}

void
print( mpz_class c )
{
  stringstream s;
  s << c;
  string t = s.str();
  for(int i=0; i<t.size(); ++i)
    addch(t[i]);
}

void
describe( word w )
{
  word data = w/256;
  for(int i=0; i<3; ++i)
    {
      switch(data%8)
	{
	case 0: print("xrb;"); break;
	case 1: print("dec;"); break;
	case 2: print("dbl;"); break;
	case 3: print("hlf;"); break;
	case 4: print("inv;"); break;
	case 5: print("xra;"); break;
	case 6: print("sqr;"); break;
	case 7: print("inc;"); break;
	}
      data/=8;
    }
}

class network
{
public:
  queue<word> bus;
  
  machine ms[256];
  
  void
  step()
  {
    word w = bus.front();
    bus.pop();
    bus.push( ms[w%256].run( w/256 ) );
  }

  

  string
  compute_msg()
  {
    word w(0);
    stringstream m;
    word colsums[16] = {0};
    for(int i=0; i<16; ++i)
      {
	int rowsum(0);
	for(int j=0; j<16; ++j)
	  {
	    word v = ms[i*16+j].value/256;
	    m << cip[v%32];

	    rowsum += v;
	    colsums[j] += v;
	    
	  }
	m << " ";
	m << cip[rowsum%32];
	m << cip[(rowsum/32)%32];
      }

    m << " ";
    for(int i=0; i<16; ++i)
      {
	m << cip[colsums[i]%32];
	m << cip[(colsums[i]/32)%32];
      }
    return m.str();
  }

  void 
  display( int draw_only = -1 )
  {
    static bool initialized(false);
    word colsums[16] = {0};

    for(int i=0; i<16; ++i)
      {
	if(!initialized)
	  {
	    attron( COLOR_PAIR(2) );
	    move(i+1,0);
	    addch( hex[i] );
	    addch( '_' );
	    move(0,i*5+3 );
	    addch( '_' );
	    addch( hex[i] );
	    attroff( COLOR_PAIR(2) );
	    
	  }
	word rowsum(0);
	for(int j=0; j<16; ++j)
	  {
	    move(i+1,j*5+2);
	    attron(COLOR_PAIR(3));
	    print( ms[i*16+j].value );
	    attroff(COLOR_PAIR(3));
	    attron(COLOR_PAIR(1));
	    addch( cip[(ms[i*16+j].value/256)%32] );
	    attroff(COLOR_PAIR(1));
	    rowsum += ms[i*16+j].value/256;
	    colsums[j] += ms[i*16+j].value/256;
	  }
	char c = cip[(rowsum)%32];
	print(" = ");
	addch( cip[ rowsum%32 ] );
	addch( cip[ (rowsum/32)%32 ] );
      }
    for(int i=0; i<16; ++i)
      {
	move(17, i*5 + 3);
	
	addch( cip[ colsums[i]%32 ] );
	addch( cip[ (colsums[i]/32)%32 ] );
      }

    move(18,2);
    print("Machine: ");
    print( bus.front()%256 );
    move(18, 20 );
    print("Data: ");
    print( bus.front()/256 );
    print(" [");
    describe( bus.front() );
    print("]");
    refresh();
    /*
    std::cout << std::dec;
    std::cout.width(0);
    std::cout << "Machine " << (bus.front()%256) << " gets ";
    std::cout << (bus.front()/256) << "\n";
    */
    initialized = true;
  }

  

public:
  network( word initial_value )
  {
    bus.push(initial_value);
  }

  network()
  {
  }
};

int 
main( int argc, char **argv )
{
  network n;

  string msg;
  std::cout << "Search for message: ";
  std::getline( std::cin, msg );
  std::cout << "'Program' (series of unsigned 16-bit integers).\n";
  std::cout << "-1 => end of input\n";
  std::cout << ": ";
  int v;
  while( std::cin >> v )
    {
      if(v==-1)
	break;
      else
	n.bus.push(v);
    }

  initscr();
  start_color();
  init_pair( 1, COLOR_GREEN, COLOR_BLACK );
  init_pair( 2, COLOR_YELLOW, COLOR_BLACK );
  init_pair( 3, COLOR_BLUE, COLOR_BLACK );
 
  //  timeout(-1);
  nodelay(stdscr,true);
  curs_set(0);
  mpz_class j(0);
  bool found = false;
  bool camera = true;
  while(!found)
    {
      //      std::cout << "Machine " << (n.bus.front()%256) << " gets ";
      //      std::cout << (n.bus.front()/256) << "\n";
      if( camera ) {
	n.display();
      }
      string s = n.compute_msg();
      move(18, 70);
      //print(s.c_str());
      stringstream ss(s);
      
      if( s.find(msg) != string::npos )
	{
	  nodelay(stdscr,false);
	  // need to redraw screen if camera is off.
	  if( !camera ) {
	    n.display();
	  }
	  print("FOUND!");
	  std::size_t p = s.find(msg);
    
	  getch();
	  break;
	}
      n.step();    
      ++j;
      
      if( camera ) {
	move( 18, 52 );
	print("Step: ");
	print(j);
      }

      int c = getch();
      if( c=='q' ) {
	break;
      } else if ( c==' ' ) {
	camera = !camera;
	if( !camera ) {
	  clear();
	  move(0,0);
	  print("Camera off. Press spacebar to reenable.");
	}
      }
    }
  
 
  endwin();
}
