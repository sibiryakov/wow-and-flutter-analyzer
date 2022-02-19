
double
process_2nd_order(register double val) {
   static double buf[4];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 3*sizeof(double));
   // use 0.00120740519032883 below for unity gain at 100% level
   iir= val * 0.001207405190260069;
   iir -= 0.9483625336008361*tmp; fir= tmp;
   iir -= -1.73410899821474*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9533938855978508*tmp; fir= tmp;
   iir -= -1.781298800713404*buf[2]; fir += buf[2]+buf[2];
   fir += iir;
   buf[3]= iir; val= fir;
   return val;
}

// Frequency-response:
//   Peak gain: 1
//   Guessed 100% gain: 1
//   Regions between half-power points (70.71% response or -3.01dB):
//     1.00006Hz -> 3150Hz  (width 3149Hz)
//   Regions between quarter-power points (50% response or -6.02dB):
//     0.688755Hz -> 3150Hz  (width 3149.31Hz)
//   Sampling rate is 6300Hz

double
process_hipass_1hz(register double val) {
   static double buf[2];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 1*sizeof(double));
   iir= val * 0.9993212972714023;
   iir -= 0.9986429017039066*tmp; fir= tmp;
   iir -= -1.998642287381703*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   buf[1]= iir; val= fir;
   return val;
}

// Frequency-response:
//   Peak gain: 1
//   Guessed 100% gain: 1
//   Regions between half-power points (70.71% response or -3.01dB):
//     0Hz -> 9.99971Hz  (width 9.99971Hz)
//   Regions between quarter-power points (50% response or -6.02dB):
//     0Hz -> 17.3204Hz  (width 17.3204Hz)
//
// Time-response:
//   Sampling rate is 6300Hz


double
process_lp_10hz(register double val) {
   static double buf[1];
   register double tmp, fir, iir;
   tmp= buf[0];
   iir= val * 0.00496195258922455;
   iir -= -0.9900760948215509*tmp; fir= tmp;
   fir += iir;
   buf[0]= iir; val= fir;
   return val;
}


// Filter descriptions:
//   BpBe2/1.2-15 == Bandpass Bessel filter, order 2, -3.01dB frequencies
//     1.2-15
//   LpBe2/200 == Lowpass Bessel filter, order 2, -3.01dB frequency 200
//   HpBe2/0.2 == Highpass Bessel filter, order 2, -3.01dB frequency 0.2
//

double
process_DIN(register double val) {
   static double buf[8];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   // use 9.894850348184627e-007 below for unity gain at 100% level
   iir= val * 9.886712475608222e-007;
   iir -= 0.9718381574433894*tmp; fir= tmp;
   iir -= -1.971551266567659*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9982440100378892*tmp; fir= tmp;
   iir -= -1.998242909436813*buf[2]; fir += buf[2]+buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.6434545131997782*tmp; fir= tmp;
   iir -= -1.591050960239724*buf[4]; fir += buf[4]+buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.9997284329050403*tmp; fir= tmp;
   iir -= -1.999728408318806*buf[6]; fir += -buf[6]-buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}


// Filter descriptions:
//   BpBe4/0.3-200 == Bandpass Bessel filter, order 4, -3.01dB frequencies
//     0.3-200
double
process_unweighted(register double val) {
   static double buf[8];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   // use 0.0003306520826394921 below for unity gain at 100% level
   iir= val * 0.0003306520826380572;
   iir -= 0.6753463035083248*tmp; fir= tmp;
   iir -= -1.591483463373453*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9997682212465883*tmp; fir= tmp;
   iir -= -1.999768186333123*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.5771462662841257*tmp; fir= tmp;
   iir -= -1.514102287557188*buf[4]; fir += buf[4]+buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.9995984565721876*tmp; fir= tmp;
   iir -= -1.999598412629212*buf[6]; fir += buf[6]+buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}

// Filter descriptions:
//   BpBe4/0.3-6 == Bandpass Bessel filter, order 4, -3.01dB frequencies
//     0.3-6
//


// Example code (functionally the same as the above code, but 
//  optimised for cleaner compilation to efficient machine code)
double
process_wow(register double val) {
   static double buf[8];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   // use 3.38643522387692e-010 below for unity gain at 100% level
   iir= val * 3.386435216458736e-010;
   iir -= 0.9889822559361133*tmp; fir= tmp;
   iir -= -1.988898714745282*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9997639015233543*tmp; fir= tmp;
   iir -= -1.999763863368945*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.9849666019626395*tmp; fir= tmp;
   iir -= -1.984903954482672*buf[4]; fir += buf[4]+buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.9995704510105757*tmp; fir= tmp;
   iir -= -1.999570400238568*buf[6]; fir += buf[6]+buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}


// Filter descriptions:
//   BpBe4/6-200 == Bandpass Bessel filter, order 4, -3.01dB frequencies
//     6-200
//

// Example code (functionally the same as the above code, but 
//  optimised for cleaner compilation to efficient machine code)
double
process_flutter(register double val) {
   static double buf[8];
   register double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   // use 0.0002980764585707285 below for unity gain at 100% level
   iir= val * 0.0002980764585582655;
   iir -= 0.6858715731999449*tmp; fir= tmp;
   iir -= -1.605649703918556*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9953215690037556*tmp; fir= tmp;
   iir -= -1.995306892110805*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.5910983651395704*tmp; fir= tmp;
   iir -= -1.532453681510474*buf[4]; fir += buf[4]+buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.9916845997627537*tmp; fir= tmp;
   iir -= -1.991665582083071*buf[6]; fir += buf[6]+buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}

