HEAD
  * leandvb is now distributed as part of leansdr.
  * Support for all DVB-S code rates.
  * Status output for third-party UIs: lock, MER, frequency offset.
  * Software AGC is always enabled. rtl-sdr HW AGC is not recommended.
  * stderr is quiet by default. Use -v -d for troubleshooting.
  * Deconvolution is now algebraic (instead of look-up table).
  * Added leansdrscan for cycling through DVB settings.
  * Added leansdrcat for debugging real-time behaviour.

2016-02-29 Preview release of leandvb
  * Code rate 1/2 only.
  * Hard-decision deconvolution (without Viterbi).
