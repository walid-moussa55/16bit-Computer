

/*
Stepper: 3 bits counter from 0 to 6, reset to 0 when R is high, increment on the falling edge of nCLK
    - inputs: nCLK, R
    - outputs: Q
*/
class Stepper{
    public:
        Stepper(): Q(0) {}
        ~Stepper() {}
        void setCLK(bool clk) {
            nCLK = !clk;
        }
        void Reset(){
            Q = 0;
        }
        void step(){
            if (nCLK) {
                Q = (Q + 1) % 7; // 3 bits counter, so it wraps around at 7
            }
        }
        short getQ() const {
            return Q;
        }
        friend std::ostream& operator<<(std::ostream& os, const Stepper& stepper) {
            os << stepper.getQ();
            return os;
        }
        
    private:
        short Q; // 3 bits output
        bool nCLK; // invert clock input
};