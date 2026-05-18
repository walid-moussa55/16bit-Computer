
/*
Counter: 16-bit counter with load, increment, decrement, and output control
    - inputs: CLK, D, JInEO
    - outputs: Q
*/
class Counter{
public:
    Counter(): _value(0), D(0), Q(0), CLK(false), J(false), In(false), E(false), O(false) {}
    ~Counter(){}
    void setCLK(bool clk){CLK = clk;}
    void Reset(){
        _value = 0;
    }
    void setD(unsigned short d){D = d;};
    void setJInEO(bool j, bool in, bool e, bool o){J = j; In = in; E = e; O = o;}
    unsigned short getQ() const {return Q;};
    unsigned short getValue() const {return _value;}
    void step(){
        if (CLK){
            if (J){
                _value = D;
            } else if(In && E){
                _value = Q - 1;
            } else if(!In && E){
                _value = Q + 1;
            }
        } else if (O){
            Q = _value;
        }
    }
    // print function that print as hexadecimal with width 4 filled with 0 with concatination with iostream using operator overloading
    friend std::ostream& operator<<(std::ostream& os, const Counter& counter) {
        os << std::hex << std::setw(4) << std::setfill('0') << counter.getValue();
        return os;
    }
private:
    bool CLK, J, In, E, O;
    unsigned short D, Q, _value;
};