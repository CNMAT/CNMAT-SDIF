/* Original author: Eric D. Scheirer, MIT Media Laboratory
 *
 * This source file has been placed in the public domain by its author(s).
 */

#ifndef _sa_bitstream_h_
#define _sa_bitstream_h_

#include <vector>
using namespace std;

// Maximum size of parsable arrays
static const int _F_SIZE = 64;

// Temporary variable for parse sizes
// (to preserve order or parse and init evaluations)
static int _F_parse;

// Temporary variables for ID parsing
static int         _F_i_id;
static float       _F_f_id;
static double      _F_d_id;
static long double _F_ld_id;
// Bitstream error reporting function
void flerror(char *s);
int samp_table[1024],samp_ct = 0;



class sa_symbol { // 'symbol' in Subclause 5.5.2
public:
    unsigned int sym;

    sa_symbol(int i) {
      sym = i;
    }

    sa_symbol() { sym  = 0; }
    
	int num() { return sym; }

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(sym, 16);
        return _F_ret;
    }
};

class sym_name {
public:
    unsigned int length;
    char name[_F_SIZE];


  sym_name() {
    length = 0;
  }
  
  sym_name(char *text) {
    length = strlen(text);
    strcpy(name,text);
  }
  
    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(length, 4);
        int _F_name_dim0, _F_name_dim0_end;
        _F_name_dim0_end = length;
        for (_F_name_dim0 = 0; _F_name_dim0 < _F_name_dim0_end; _F_name_dim0++) {
            _F_bs.putbits(name[_F_name_dim0], 8);
        }
        return _F_ret;
    }
};

#define _F_SIZE 8192


class sa_symtable {  // 'symtable' in Clause 5.5.2
public:
    unsigned int length;
    vector<sym_name,malloc_alloc> name;


  public: sa_symtable() {
  length = 0;
  name.clear();
  }
  
  public: sa_symbol add(char *text) {
    int i= 0;
    while (i < length && strcmp((const char *)name[i].name,text)) i++;

    if (i == length) {
      sym_name n(text);
      name.push_back(n);
      length++;
    }
    sa_symbol sym(i);
    return sym;
  }

  public: 
	  char *ref(sa_symbol s) {
		  int i;
		  char *st;
		  
		  if (this)
			  for (i=0;i!=length;i++)
				  if (i == s.sym)
					  return name[i].name;
				  
				  st = new char[40];
				  sprintf(st,"_sym_%d",s.sym);
				  
				  return st;
	  }
	  
		  
		  
  public: 
	  virtual int put(Bitstream& _F_bs) {
		  int _F_ret=0;
		  _F_bs.putbits(length, 16);
		  int _F_name_dim0, _F_name_dim0_end;
		  _F_name_dim0_end = length;
		  for (_F_name_dim0 = 0; _F_name_dim0 < _F_name_dim0_end; _F_name_dim0++) {
			  _F_ret += name[_F_name_dim0].put(_F_bs);
		  }
		  return _F_ret;
	  }
};

#undef _F_SIZE
#define _F_SIZE 256


class orch_token {
public:
    int done;
    unsigned int token;
    sa_symbol sym;
    float val;
    unsigned int ival;
    int length;
    char str[_F_SIZE];

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(token, 8);
        switch (token)
            {
            case 0xF0:
                _F_ret += sym.put(_F_bs);
                break;
            case 0xF1:
                _F_bs.putfloat(val);
                break;
            case 0xF2:
               _F_bs.putbits(ival, 32);
                break;
            case 0xF3:
                _F_bs.putbits(length, 8);
                int _F_str_dim0, _F_str_dim0_end;
                _F_str_dim0_end = length;
                for (_F_str_dim0 = 0; _F_str_dim0 < _F_str_dim0_end; _F_str_dim0++) {
                    _F_bs.putbits(str[_F_str_dim0], 8);
                }
                break;
	    case 0xF4:
	      _F_bs.putbits(ival,8);
	      break;
            case 0xFF:
                done=1;
                break;
            }
        return _F_ret;
    }
};

#undef _F_SIZE
#define _F_SIZE 8192


class orc_file {
public:
    unsigned int length;
    vector<orch_token,malloc_alloc> data;

  orc_file() { 
	  length = 0; 
	  data.clear();
  }

  void add(orch_token ot) {
    data.push_back(ot);
    length++;
  }
  
    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(length, 16);
        int _F_data_dim0, _F_data_dim0_end;
        _F_data_dim0_end = length;
        for (_F_data_dim0 = 0; _F_data_dim0 < _F_data_dim0_end; _F_data_dim0++) {
            _F_ret += data[_F_data_dim0].put(_F_bs);
        }
        return _F_ret;
    }
};

#undef _F_SIZE
#define _F_SIZE 256


class instr_event {
public:
    unsigned int has_label;
    sa_symbol label;
    sa_symbol iname_sym;
    float dur;
    unsigned int num_pf;
    float pf[_F_SIZE];

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(has_label, 1);
        if (has_label)
            _F_ret += label.put(_F_bs);
        _F_ret += iname_sym.put(_F_bs);
        _F_bs.putfloat(dur);
        _F_bs.putbits(num_pf, 8);
        int _F_pf_dim0, _F_pf_dim0_end;
        _F_pf_dim0_end = num_pf;
        for (_F_pf_dim0 = 0; _F_pf_dim0 < _F_pf_dim0_end; _F_pf_dim0++) {
            _F_bs.putfloat(pf[_F_pf_dim0]);
        }
        return _F_ret;
    }
};

class control_event {
public:
    unsigned int has_label;
    sa_symbol label;
    sa_symbol varsym;
    float value;

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(has_label, 1);
        if (has_label)
            _F_ret += label.put(_F_bs);
        _F_ret += varsym.put(_F_bs);
        _F_bs.putfloat(value);
        return _F_ret;
    }
};

class table_event {
public:
    sa_symbol tname;
    unsigned int tgen;
	unsigned int destroy;
    unsigned int refers_to_sample;
    sa_symbol table_sym;
    unsigned int num_pf;
    float *pf;

	table_event() {
		pf = NULL;
	}

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_ret += tname.put(_F_bs);
		_F_bs.putbits(destroy,1);
		if (!destroy) {
			_F_bs.putbits(tgen, 8);
			_F_bs.putbits(refers_to_sample, 1);
			if (refers_to_sample)
				_F_ret += table_sym.put(_F_bs);
			_F_bs.putbits(num_pf, 16);
			int _F_pf_dim0, _F_pf_dim0_end;
			_F_pf_dim0_end = num_pf;
			for (_F_pf_dim0 = 0; _F_pf_dim0 < _F_pf_dim0_end; _F_pf_dim0++) {
				_F_bs.putfloat(pf[_F_pf_dim0]);
			}
		}
        return _F_ret;
    }
};


class end_event {
  /* This is correct; no data, don't do anything. */
public:

    public: virtual int put(Bitstream& _F_bs) {
      return 0;
    }
};


class tempo_event {
public:
    float tempo;

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putfloat(tempo);
        return _F_ret;
    }
};

class score_line {
public:
    int has_time;
    int use_if_late;
    int priority;
    float time;
    unsigned int type;
    instr_event inst;
    control_event control;
    table_event table;
    tempo_event tempo;
    end_event end;

    public: 
	
		score_line() {
			has_time = 0;
			use_if_late = 0;
			priority = 0;
			time = 0;
			type = 0;
		}

	virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
	_F_bs.putbits(has_time,1);
	if (has_time) {
	  _F_bs.putbits(use_if_late,1);
	  _F_bs.putfloat(time);
	}
	_F_bs.putbits(priority,1);
	_F_bs.putbits(type, 3);
	
        switch (type)
            {
            case 0:
                _F_ret += inst.put(_F_bs);
                break;
            case 1:
                _F_ret += control.put(_F_bs);
                break;
            case 2:
                _F_ret += table.put(_F_bs);
                break;
            case 5:
                _F_ret += tempo.put(_F_bs);
                break;
            case 4:
                _F_ret += end.put(_F_bs);
                break;
            }
        return _F_ret;
    }
};


class score_file {
public:
    unsigned int num_lines;
    vector<score_line *,malloc_alloc>lines;

public: score_file() { num_lines = 0;  }
    
  public: void add(score_line *s) {
    num_lines++;
    lines.push_back(s);
  }
 

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(num_lines, 20);
        int _F_lines_dim0, _F_lines_dim0_end;
        _F_lines_dim0_end = num_lines;
        for (_F_lines_dim0 = 0; _F_lines_dim0 < _F_lines_dim0_end; _F_lines_dim0++) {
            _F_ret += lines[_F_lines_dim0]->put(_F_bs);
        }
        return _F_ret;
    }
};


class midi_event {
   
public:
 unsigned int length;
  vector <unsigned char,malloc_alloc> data;

  midi_event(void) {
    length = 0;
	data.clear();
  }
  
    public: virtual int put(Bitstream& _F_bs) {
        int i;
        int _F_ret=0;

	_F_bs.putbits(length,24);
	for (i=0;i!=length;i++)
	  _F_bs.putbits(data[i],8);
	
        return _F_ret;
    }
};

class midi_file {
public:
    unsigned int length;
    vector <unsigned char,malloc_alloc> data;

    public: virtual int put(Bitstream& _F_bs) {
        int _F_ret=0;
        _F_bs.putbits(length, 32);
        int _F_data_dim0, _F_data_dim0_end;
        _F_data_dim0_end = length;
        for (_F_data_dim0 = 0; _F_data_dim0 < _F_data_dim0_end; _F_data_dim0++) {
            _F_bs.putbits(data[_F_data_dim0], 8);
        }
        return _F_ret;
    }
};

class bit_data {  // not part of the bitstream -- used for decoding
  public:
  int num_orc, num_sco, has_sym, num_midi;
  orc_file *all_orc[20];
  score_file *all_score[20];
  midi_file *all_midi[20];

  void add_orc(orc_file *o) {
    all_orc[num_orc++] = o;
  }

  void add_score(score_file *s) {
    all_score[num_sco++] = s;
  }

  void add_midi(midi_file *m) {
    all_midi[num_midi++] = m;
  }

  /* should do this for SBF as well */
  
} all;
 
class sample {
public:
    sa_symbol sample_name_sym;
    char fn[20]; 
    unsigned int length;
    unsigned int has_srate;
    unsigned int srate;
    unsigned int has_loop;
    unsigned int loopstart;
    unsigned int loopend;
    unsigned int has_base;
    float basecps;
    unsigned int float_sample;
    vector<float,malloc_alloc> fs_data;
    vector<int,malloc_alloc> s_data;
    
 public: void save(char *);

  sample() {
    length = 0;
	fn[0] = '\0';
	has_srate = 0;
	srate = 0;
	has_loop = 0;
	loopstart=0;
	loopend = 0;
	has_base = 0;
	basecps = 0;
	float_sample = 0;
	fs_data.clear();
	s_data.clear();
   }
      
  virtual int put(Bitstream& _F_bs) {
    int _F_ret=0;
    _F_ret += sample_name_sym.put(_F_bs);
    _F_bs.putbits(length, 24);
    _F_bs.putbits(has_srate, 1);
    if (has_srate)
      _F_bs.putbits(srate, 17);
    _F_bs.putbits(has_loop, 1);
    if (has_loop)
      {
	_F_bs.putbits(loopstart, 24);
	_F_bs.putbits(loopend, 24);
      }
    _F_bs.putbits(has_base, 1);
    if (has_base)
      _F_bs.putfloat(basecps);
    _F_bs.putbits(float_sample, 1);
    if (float_sample)
      {
	int _F_fs_data_dim0, _F_fs_data_dim0_end;
	//	_F_bs.align(8);
	_F_fs_data_dim0_end = length;
	for (_F_fs_data_dim0 = 0;
	     _F_fs_data_dim0 < _F_fs_data_dim0_end;
	     _F_fs_data_dim0++) {
	  _F_bs.putfloat(fs_data[_F_fs_data_dim0]);
	}
      }
    else
      {
	int _F_s_data_dim0, _F_s_data_dim0_end;
	//	_F_bs.align(8);
	_F_s_data_dim0_end = length;
	for (_F_s_data_dim0 = 0;
	     _F_s_data_dim0 < _F_s_data_dim0_end;
	     _F_s_data_dim0++) {
	  _F_bs.putbits((unsigned short)s_data[_F_s_data_dim0], 16);
	}
      }
    return _F_ret;
  }
};

class sbf {
public:
  
  int length;
  vector<char,malloc_alloc> data;
  
  sbf() {
    length = 0;
   }
  
public:
  virtual int put(Bitstream& _F_bs) {
    int _F_ret=0,i;
    
    _F_bs.putbits(length,32);
    for (i=0;i!=length;i++)
      _F_bs.putbits(data[i],8);
    return _F_ret;
  }
};


class StructuredAudioSpecificConfig { // the bitstream header
public:
    unsigned int more_data;
    unsigned int chunk_type;
    orc_file *orc;
    score_file *score;
    midi_file *SMF;
    sample *samp;
    sa_symtable *sym;
    sbf *sample_bank;

public: virtual int put(Bitstream& _F_bs) {
    int _F_ret=0;
    _F_parse = 1;
    more_data = 1;
    _F_bs.putbits(more_data, _F_parse);
    while (more_data)
        {
            _F_bs.putbits(chunk_type, 3);
            switch (chunk_type)
                {
                case 0:
                    _F_ret += orc->put(_F_bs);
                    break;
                case 1:
                    _F_ret += score->put(_F_bs);
                    break;
                case 2:
                    _F_ret += SMF->put(_F_bs);
                    break;
                case 3:
                    _F_ret += samp->put(_F_bs);
                    break;
                case 4:
                    _F_ret += sample_bank->put(_F_bs);
                    break;
                case 5:
                    _F_ret += sym->put(_F_bs);
                    break;
                }
            _F_bs.putbits(more_data, 1);
        }
    return _F_ret;
}
};


class au_event {
public:
	score_line score_ev;
	midi_event midi_ev;
	sample samp;
	int event_type;

	au_event() {
		event_type = 0;
	}
} ;


class SA_access_unit {
public:
  unsigned int num_events;
  float dts;
  unsigned int out_of_data;
  vector<au_event *,malloc_alloc> events;

public:
	SA_access_unit() {
		num_events = 0;
		dts = 0;
		out_of_data = 0;
		events.clear();
	}

  virtual int put(Bitstream& _F_bs) {
    int _F_ret=0;
	int i;
   
	_F_ret += (int)_F_bs.putfloat(dts);    // VIOLATION

	for (i=0;i!=num_events;i++) {
		
		_F_ret += _F_bs.putbits(1,1); // more_data

		_F_ret += _F_bs.putbits(events[i]->event_type,2);

		switch (events[i]->event_type) {

		case 0:
			_F_ret += events[i]->score_ev.put(_F_bs);
			break;
		case 1:
			_F_ret += events[i]->midi_ev.put(_F_bs);
			break;
		case 2:
			_F_ret += events[i]->samp.put(_F_bs);
			break;
		}
    
	}
	_F_ret += _F_bs.putbits(0,1); // no more data

    return _F_ret;
  }

};

bool operator<(SA_access_unit a, SA_access_unit b) {
	// NB backwards since we want lowest "priority" (time) at the front
	return a.dts > b.dts;
}

#endif /* ! _sa_bitstream_h_ */
