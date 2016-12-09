#ifndef QHARMONICPROCESSOR_H
#define QHARMONICPROCESSOR_H

#include <QObject>
#include "fftw3.h"
#include "ap.h" // ALGLIB types
#include "dataanalysis.h" // ALGLIB functions

#define BOTTOM_LIMIT 0.8 // in s^-1, it is 48 bpm
#define TOP_LIMIT 3.5 // in s^-1, it is 210 bpm
#define SNR_TRESHOLD 2.0 // in most cases this value is suitable when (m_BufferLength == 256)
#define HALF_INTERVAL 2 // defines the number of averaging indexes when frequency is evaluated, this value should be >= 1
#define DIGITAL_FILTER_LENGTH 9 // in counts

#define BREATH_TOP_LIMIT 0.5 // in s^-1, it is 30 rpm
#define BREATH_BOTTOM_LIMIT 0.2 // in s^-1, it is 12 rpm
#define BREATH_HALF_INTERVAL 2 // it will be (value * 2 + 1)
#define BREATH_SNR_TRESHOLD 2.0

#define PRUNING_SKO_COEFF 3
#define DEFAULT_NORMALIZATION_INTERVAL 15
#define DEFAULT_BREATH_NORMALIZATION_INTERVAL 26
#define DEFAULT_BREATH_AVERAGE 16
#define DEFAULT_BREATH_STROBE 3

class QHarmonicProcessor : public QObject
{
    Q_OBJECT
public:
    explicit QHarmonicProcessor(QObject *parent = NULL, quint16 length_of_data = 221, quint16 length_of_buffer = 221 );
    ~QHarmonicProcessor();
    enum ColorChannel { Red, Green, Blue, RGB, Experimental };
    enum XMLparserError { NoError, FileOpenError, FileExistanceError, ReadError, ParseFailure };
    enum SexID { Male, Female };
    enum TwoSideAlpha { FiftyPercents, TwentyPercents, TenPercents, FivePercents, TwoPercents };

signals:
    void heartSignalUpdated(const qreal * pointer_to_vector, quint16 length_of_vector);
    void heartSpectrumUpdated(const qreal * pointer_to_vector, quint16 length_of_vector);
    void TimeUpdated(const qreal * pointer_to_vector, quint16 length_of_vector);
    void heartRateUpdated(qreal freq_value, qreal snr_value, bool reliable_data_flag);
    void PCAProjectionUpdated(const qreal * ppointer_to_vector, quint16 length_of_vector);
    void BinaryOutputUpdated(const qreal *pointer_to_vector, quint16 length_of_vector);
    void CurrentValues(qreal signalValue, qreal meanRed, qreal meanGreen, qreal meanBlue);
    void heartTooNoisy(qreal snr_value);

    void snrUpdated(quint32 id, qreal value);    // signal for mapping
    void vpgUpdated(quint32 id, qreal value);   // signal for mapping
    void svpgUpdated(quint32 id, qreal value);  // signal for mapping
    void bvpgUpdated(quint32 id, qreal value);  // signal for mapping
    void amplitudeUpdated(quint32 id, qreal value); // signal for mapping

    void breathSignalUpdated(const qreal *pointer, quint16 length);
    void breathSpectrumUpdated(const qreal *pointer, quint16 length);
    void breathRateUpdated(qreal freq_value, qreal snr_value);
    void breathTooNoisy(qreal snr_value);
    void breathSnrUpdated(quint32 id, qreal snr_value);
    void measurementsUpdated(qreal heart_rate, qreal heart_snr, qreal breath_rate, qreal breath_snr);

    void spO2Updated(const qreal value);

public slots:
    void EnrollData(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, double time);
    void computeHeartRate(); // computes Heart Rate by means of frequency analysis
    void computeBreathRate(); // computes Breath Rate by means of frequency analysis
    void computeSPO2(quint16 index); // computes SPO2 by means of frequency analysis and ratio of the ration method
    void CountFrequency(); //simple time domain algorithm for HeartRate evaluation, not very accurate
    void setPCAMode(bool value); // controls PCA alignment
    void switchColorMode(int value); // controls colors enrollment
    int  loadWarningRates(const char *fileName, SexID sex, int age, TwoSideAlpha alpha);
    void setID(quint32 value); // use it to set ID, it is used for QHarmonicMapper internal logic management
    void setEstiamtionInterval(int value); // use it to set m_estimationInterval property value
    void setBreathStrobe(int value);
    void setBreathAverage(int value);
    void setBreathCNInterval(int value);
    unsigned int getDataLength() const;
    unsigned int getBufferLength() const;
    quint16 getEstimationInterval() const;
    quint16 getBreathStrobe() const;
    quint16 getBreathAverage() const;
    quint16 getBreathCNInterval() const;
    void setSnrControl(bool value);
    void setPruning(bool value);


private:
    qreal *v_HeartSignal;  //a pointer to centered and normalized data (typedefinition from fftw3.h, a single precision complex float number type)
    qreal *v_HeartCNSignal; // a pointer to input counts history, for digital filtration
    fftw_complex *v_HeartSpectrum;  // a pointer to an array for FFT-spectrum
    qreal m_HeartSNR; // a variable for signal-to-noise ratio estimation storing
    qreal *v_RawCh1; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    qreal *v_RawCh2; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    qreal *v_HeartForFFT; //a pointer to data prepared for FFT
    qreal *v_HeartAmplitude; // stores amplitude spectrum
    qreal *v_HeartTime; //a pointer to an array for frame periods storing (values in milliseconds thus unsigned int)
    qreal m_HeartRate; //a variable for storing a last evaluated frequency of the 'strongest' harmonic
    unsigned int curpos; //a current position I meant
    unsigned int m_DataLength; //a length of data array
    unsigned int m_BufferLength; //a lenght of sub data array for FFT (m_BufferLength should be <= m_DataLength)
    bool f_PCA; // this flag controls whether computeHeartRate use ordinary computation or PCA alignment, value is controlled by set_f_PCA(...)
    fftw_plan m_HeartPlan; // a plan for FFT evaluation

    ColorChannel m_ColorChannel; // determines which color channel is enrolled by WriteToDataOneColor(...) method
    qreal *v_BinaryOutput; // a pointer to a vector of digital filter output
    qreal *v_SmoothedSignal; // for intermediate result storage
    qreal v_Derivative[2]; // to store two close counts from digital derivative
    quint8 m_zerocrossing; // controls zero crossings of the first derivative
    qint16 m_PulseCounter; // will store the number of pulse waves for averaging m_HeartRate estimation
    double m_leftThreshold; // a bottom threshold for warning about high pulse value
    double m_rightTreshold; // a top threshold for warning aboul low pulse value
    qreal m_output; // a variable for v_BinaryOutput control, it should take values 1.0 or -1.0

    alglib::real_2d_array PCA_RAW_RGB; // a container for PCA analysis
    alglib::real_1d_array PCA_Variance; // array[0..2] - variance values corresponding to basis vectors
    alglib::real_2d_array PCA_Basis; // array[0..2,0..2], whose columns will store basis vectors
    alglib::ae_int_t PCA_Info; // PCA result code

    quint16 loop(qint16) const; //a function that return a loop-index
    quint16 loopInput(qint16) const; //a function that return a loop-index
    quint16 loopBuffer(qint16) const; //a function that return a loop-index
    quint8 loopOnTwo(qint16 difference) const;

    quint32 m_ID;
    quint16 m_estimationInterval; // stores the number of counts that will be used to evaluate mean and sko estimations
    bool m_HeartSNRControlFlag; //

    qreal *v_RawBreathSignal; // stores slow changes in VPG, not centered and not normalized
    qreal *v_BreathSignal; // to store a slow waves and evaluate a breath rate
    qreal *v_BreathTime; // to store a time counters for breath signal
    qreal *v_BreathForFFT;
    qreal *v_BreathAmplitude;
    fftw_plan m_BreathPlan;
    fftw_complex *v_BreathSpectrum;
    qreal m_BreathRate; // to store a breath rate measurement
    quint16 m_BreathStrobe;
    quint16 m_BreathStrobeCounter;
    quint16 m_BreathCurpos;
    quint16 m_BreathAverageInterval;
    quint16 m_BreathCNInterval;
    qreal m_BreathSNR;

    fftw_plan m_BluePlan;
    fftw_complex *v_BlueSpectrum;
    qreal *v_BlueForFFT;
    fftw_plan m_RedPlan;
    fftw_complex *v_RedSpectrum;
    qreal *v_RedForFFT;
    qreal m_SPO2;

    bool m_pruningFlag;
};

// inline, for speed, must therefore reside in header file
inline quint16 QHarmonicProcessor::loop(qint16 difference) const
{
    return ((m_DataLength + (difference % m_DataLength)) % m_DataLength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
inline quint16 QHarmonicProcessor::loopInput(qint16 difference) const
{
    return ((DIGITAL_FILTER_LENGTH + (difference % DIGITAL_FILTER_LENGTH)) % DIGITAL_FILTER_LENGTH);
}
//---------------------------------------------------------------------------
inline quint16 QHarmonicProcessor::loopBuffer(qint16 difference) const
{
    return ((m_BufferLength + (difference % m_BufferLength)) % m_BufferLength);
}
//---------------------------------------------------------------------------
inline quint8 QHarmonicProcessor::loopOnTwo(qint16 difference) const
{
    return ((2 + (difference % 2)) % 2);
}

//---------------------------------------------------------------------------
#endif // QHARMONICPROCESSOR_H
