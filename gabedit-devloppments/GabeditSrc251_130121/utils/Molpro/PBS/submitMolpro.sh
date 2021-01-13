#!/bin/sh
###################################################################
#                                                                 #
#  Script ksh pour lancer un calcul Molpro en batch               #
#  Auteur:  A-R Allouche allouche@lasim.univ-lyon1.fr             #
#  LASIM, Villeurbanne                                            #
#  Ecriture : September 2004                                      #
#                                                                 #
###################################################################

jobtime=$1 ; [ 0 = $# ] || shift
filename=$1 ; [ 0 = $# ] || shift

if [ -z "$filename" ] || [ -z "$jobtime" ]
then
   clear
   echo " " 
   echo "==========================================="
   echo "                 submitMolpro "
   echo " Pour toutes questions contacter : "
   echo " A.R. Allouche allouche@lasim.univ-lyon1.fr"
   echo "==========================================="
   echo "vous devez fournir :"
   echo "      - le temps de calcul( hh:mm:ss)"
   echo "      - le nom du fichier(y compris l'extension .com)"
   echo "-------------------------------------------"
   echo "Exemple:"
   echo "       submitMolpro 2:00:00 h2.com "
   echo "       fichier de donnees         : h2.com"
   echo "       Temps de calcul(maximum)   : 2 heurs"
   echo "==========================================="
   echo " " 
   exit 1
fi
filecom=$filename
filename=${filecom%.com}
DEFAULTDIR=`pwd`
if test ! -s "$filecom"
then
   echo " " 
   echo "============================================"
   echo "le fichier $filecom est introuvable(ou vide)"
   echo "============================================"
   echo " " 
   exit 2
fi

endof=EOF
cat > $filename.pbs <<EOF
#!/bin/sh
#PBS -j oe
#PBS -N $filename
#PBS -l walltime=$jobtime
#PBS -o $filename.pbsout
/data/logiciels/Molpro/bin/molpro "$DEFAULTDIR/$filecom" 
EOF

qsub $filename.pbs
echo " " 
echo "==================================================================="
echo " " 
echo "Un fichier $filename.pbs a ete cree pour lancer le calcul"
echo "Ne pas effacer ce fichier avant la fin du calcul"
echo " " 
echo "-------------------------------------------------------------------"
echo " " 
echo "pour avoir des informations sur vos job taper : qstat "
echo " " 
echo "pour detruire un job taper : qdel num.master0.alin"
echo "   num.master0.alin est le Job Id fourni par qstat" 
echo " " 
echo "==================================================================="
echo " " 
