#!./csh -f
#  03 Feb 05 - A.R. ALLOUCHE
#
set TARGET=sockets
#
set JOB=$1          
set VERNO=$2        
set NCPUS=1        
set NNODES=1        

set SCR=$PWD\\scratch
set SCK=$PWD\\temp
set HOSTLIST=$3

if ((-x $PWD/temp) && (-x $PWD/scratch)) then      
 else
 echo " Error: /temp and /scratch folders are missing "
 echo "  please create them in WinGAMESS' root folder;"
 echo "   gabeditRunScriptGamess.csh exiting ..     "
exit
endif

if (null$VERNO == null) set VERNO=05
if (null$NNODES == null) set NNODES=1

if ($JOB:r.inp == $JOB) set JOB=$JOB:r 

#  file assignments.
unset echo
setenv GMSPATH $PWD
setenv GMSJOBNAME $JOB
setenv  master $HOSTLIST
setenv ERICFMT $GMSPATH/ericfmt.dat
setenv  EXTBAS /dev/null
setenv IRCDATA $SCK/$JOB.irc
setenv   INPUT $SCR/$JOB.F05
setenv   PUNCH $SCK/$JOB.dat
setenv  AOINTS $SCR/$JOB.F08
setenv  MOINTS $SCR/$JOB.F09
setenv DICTNRY $SCR/$JOB.F10
setenv DRTFILE $SCR/$JOB.F11
setenv CIVECTR $SCR/$JOB.F12
setenv CASINTS $SCR/$JOB.F13
setenv  CIINTS $SCR/$JOB.F14
setenv  WORK15 $SCR/$JOB.F15
setenv  WORK16 $SCR/$JOB.F16
setenv CSFSAVE $SCR/$JOB.F17
setenv FOCKDER $SCR/$JOB.F18
setenv  WORK19 $SCR/$JOB.F19
setenv  DASORT $SCR/$JOB.F20
setenv DFTINTS $SCR/$JOB.F21
setenv DFTGRID $SCR/$JOB.F22
setenv  JKFILE $SCR/$JOB.F23
setenv  ORDINT $SCR/$JOB.F24
setenv  EFPIND $SCR/$JOB.F25
setenv PCMDATA $SCR/$JOB.F26                                     
setenv PCMINTS $SCR/$JOB.F27
setenv SVPWRK1 $SCR/$JOB.F26
setenv SVPWRK2 $SCR/$JOB.F27
setenv  MLTPL  $SCR/$JOB.F28
setenv  MLTPLT $SCR/$JOB.F29
setenv  DAFL30 $SCR/$JOB.F30
setenv  SOINTX $SCR/$JOB.F31
setenv  SOINTY $SCR/$JOB.F32
setenv  SOINTZ $SCR/$JOB.F33
setenv  SORESC $SCR/$JOB.F34
setenv   SIMEN $SCK/$JOB.simen
setenv  SIMCOR $SCK/$JOB.simcor
setenv GCILIST $SCR/$JOB.F37
setenv HESSIAN $SCR/$JOB.F38
setenv SOCCDAT $SCR/$JOB.F40
setenv  AABB41 $SCR/$JOB.F41
setenv  BBAA42 $SCR/$JOB.F42
setenv  BBBB43 $SCR/$JOB.F43
setenv  MCQD50 $SCR/$JOB.F50
setenv  MCQD51 $SCR/$JOB.F51
setenv  MCQD52 $SCR/$JOB.F52
setenv  MCQD53 $SCR/$JOB.F53
setenv  MCQD54 $SCR/$JOB.F54
setenv  MCQD55 $SCR/$JOB.F55
setenv  MCQD56 $SCR/$JOB.F56
setenv  MCQD57 $SCR/$JOB.F57
setenv  MCQD58 $SCR/$JOB.F58
setenv  MCQD59 $SCR/$JOB.F59
setenv  MCQD60 $SCR/$JOB.F60
setenv  MCQD61 $SCR/$JOB.F61
setenv  MCQD62 $SCR/$JOB.F62
setenv  MCQD63 $SCR/$JOB.F63
setenv  MCQD64 $SCR/$JOB.F64
setenv NMRINT1 $SCR/$JOB.F61
setenv NMRINT2 $SCR/$JOB.F62
setenv NMRINT3 $SCR/$JOB.F63
setenv NMRINT4 $SCR/$JOB.F64
setenv NMRINT5 $SCR/$JOB.F65
setenv NMRINT6 $SCR/$JOB.F66
setenv DCPHFH2 $SCR/$JOB.F67
setenv DCPHF21 $SCR/$JOB.F68
setenv   GVVPT $SCR/$JOB.F69
setenv  CCREST $SCR/$JOB.F70
setenv  CCDIIS $SCR/$JOB.F71
setenv  CCINTS $SCR/$JOB.F72
setenv CCT1AMP $SCR/$JOB.F73
setenv CCT2AMP $SCR/$JOB.F74
setenv CCT3AMP $SCR/$JOB.F75
setenv    CCVM $SCR/$JOB.F76
setenv    CCVE $SCR/$JOB.F77
setenv CCQUADS $SCR/$JOB.F78
setenv QUADSVO $SCR/$JOB.F79
setenv EOMSTAR $SCR/$JOB.F80
setenv EOMVEC1 $SCR/$JOB.F81
setenv EOMVEC2 $SCR/$JOB.F82
setenv  EOMHC1 $SCR/$JOB.F83
setenv  EOMHC2 $SCR/$JOB.F84
setenv EOMHHHH $SCR/$JOB.F85
setenv EOMPPPP $SCR/$JOB.F86
setenv EOMRAMP $SCR/$JOB.F87
setenv EOMRTMP $SCR/$JOB.F88
setenv EOMDG12 $SCR/$JOB.F89
setenv    MMPP $SCR/$JOB.F90
setenv   MMHPP $SCR/$JOB.F91
setenv MMCIVEC $SCR/$JOB.F92
setenv MMCIVC1 $SCR/$JOB.F93
setenv MMCIITR $SCR/$JOB.F94
setenv  EOMVL1 $SCR/$JOB.F95
setenv  EOMVL2 $SCR/$JOB.F96
setenv EOMLVEC $SCR/$JOB.F97
setenv  EOMHL1 $SCR/$JOB.F98
setenv  EOMHL2 $SCR/$JOB.F99
setenv  OLI201 $SCR/$JOB.F201
setenv  OLI202 $SCR/$JOB.F202
setenv  OLI203 $SCR/$JOB.F203
setenv  OLI204 $SCR/$JOB.F204
setenv  OLI205 $SCR/$JOB.F205
setenv  OLI206 $SCR/$JOB.F206
setenv  OLI207 $SCR/$JOB.F207
setenv  OLI208 $SCR/$JOB.F208
setenv  OLI209 $SCR/$JOB.F209
setenv  OLI210 $SCR/$JOB.F210
setenv  OLI211 $SCR/$JOB.F211
setenv  OLI212 $SCR/$JOB.F212
setenv  OLI213 $SCR/$JOB.F213
setenv  OLI214 $SCR/$JOB.F214
setenv  OLI215 $SCR/$JOB.F215
setenv  OLI216 $SCR/$JOB.F216
setenv  OLI217 $SCR/$JOB.F217
setenv  OLI218 $SCR/$JOB.F218
setenv  OLI219 $SCR/$JOB.F219
setenv  OLI220 $SCR/$JOB.F220
setenv  OLI221 $SCR/$JOB.F221
setenv  OLI222 $SCR/$JOB.F222
setenv  OLI223 $SCR/$JOB.F223
setenv  OLI224 $SCR/$JOB.F224
setenv  OLI225 $SCR/$JOB.F225
setenv  OLI226 $SCR/$JOB.F226
setenv  OLI227 $SCR/$JOB.F227
setenv  OLI228 $SCR/$JOB.F228
setenv  OLI229 $SCR/$JOB.F229
setenv  OLI230 $SCR/$JOB.F230
setenv  OLI231 $SCR/$JOB.F231
setenv  OLI232 $SCR/$JOB.F232
setenv  OLI233 $SCR/$JOB.F233
setenv  OLI234 $SCR/$JOB.F234
setenv  OLI235 $SCR/$JOB.F235
setenv  OLI236 $SCR/$JOB.F236
setenv  OLI237 $SCR/$JOB.F237
setenv  OLI238 $SCR/$JOB.F238
setenv  OLI239 $SCR/$JOB.F239
setenv  VB2000PATH $GMSPATH/VB2000

#
   if ((-x $GMSPATH/gamess.$VERNO.exe) && (-x $GMSPATH/ddikick.exe)) then
   else
      echo The GAMESS executable gamess.$VERNO.exe or else
      echo the DDIKICK executable ddikick.exe
      echo could not be found in directory $GMSPATH,
      echo or else they did not properly link to executable permission.
      exit 8
   endif
#
#        OK, now we are ready to execute!
#    The kickoff program initiates GAMESS process(es) on all CPUs/nodes.
#
   if ((-x $GMSPATH/gamess.$VERNO.exe) && (-x $GMSPATH/ddikick.exe)) then
      set echo
      $GMSPATH/ddikick.exe $GMSPATH/gamess.$VERNO.exe $JOB \
          -ddi $NNODES $NCPUS $HOSTLIST \
          -scr $SCR < /dev/null
      unset echo
   else
      echo The GAMESS executable gamess.$VERNO.exe or else
      echo the DDIKICK executable ddikick.exe
      echo could not be found in directory $GMSPATH,
      echo or else they did not properly link to executable permission.
      exit 8
   endif
echo ----- accounting info -----
exit
