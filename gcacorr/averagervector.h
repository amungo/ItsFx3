#ifndef AVERAGERVECTOR
#define AVERAGERVECTOR

template< class T >
class Averager {
public:
    Averager( int len, int avg_cnt ) :
        nPnt( len ),
        nAvg( avg_cnt ),
        idx( 0 ),
        first_time( true )
    {
        buf = new T*[ nAvg ];
        for ( int i = 0; i < nAvg; i++ ) {
            buf[ i ] = new T[ nPnt ];
        }
        outbuf = new T[ nPnt ];
    }

    ~Averager() {
        for ( int i = 0; i < nAvg; i++ ) {
            delete [] buf[ i ];
        }
        delete buf;
        delete outbuf;
    }

    void PushData( const T* data ) {
        if ( !first_time ) {
            T* p = buf[ idx ];
            idx = ( idx + 1 ) % nAvg;
            for ( int i = 0; i < nPnt; i++ ) {
                p[ i ] = data[ i ];
            }
        } else {
            first_time = false;
            for ( int k = 0; k < nAvg; k++ ) {
                T* p = buf[ k ];
                for ( int i = 0; i < nPnt; i++ ) {
                    p[ i ] = data[ i ];
                }
            }
        }
    }

    void GetData( T* dst, bool inverted = false ) const {
        if ( inverted ) {
            for ( int i = 0; i < nPnt; i++ ) {
                dst[ i ] = 0;
                for ( int k = 0; k < nAvg; k++ ) {
                    dst[ i ] = dst[ i ] + buf[k][nPnt - i - 1] * ( 1.0f / nAvg );
                }
            }
        } else {
            for ( int i = 0; i < nPnt; i++ ) {
                dst[ i ] = 0;
                for ( int k = 0; k < nAvg; k++ ) {
                    dst[ i ] = dst[ i ] + buf[k][i] * ( 1.0f / nAvg );
                }
            }
        }
    }

    const T* GetData( bool inverted = false ) const {
        GetData( outbuf, inverted );
        return outbuf;
    }

private:
    T** buf;
    T* outbuf;
    int nPnt;
    int nAvg;
    int idx;

    bool first_time;
};

#endif // AVERAGERVECTOR

