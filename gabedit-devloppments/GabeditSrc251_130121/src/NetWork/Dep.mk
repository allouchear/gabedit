Ftp.o: Ftp.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/Utils.h ../Common/Status.h
Process.o: Process.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/Utils.h \
 ../Utils/UtilsInterface.h ../Utils/Constants.h \
 ../Utils/GabeditTextEdit.h ../Common/Run.h ../Common/Windows.h \
 ../NetWork/RemoteCommand.h ../NetWork/Process.h
Rsh.o: Rsh.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/Utils.h
Ssh.o: Ssh.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/Utils.h
Scp.o: Scp.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/UtilsInterface.h \
 ../Utils/Utils.h ../Common/Status.h
FilesTransfer.o: FilesTransfer.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../NetWork/Ftp.h ../NetWork/Scp.h
RemoteCommand.o: RemoteCommand.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../NetWork/Rsh.h ../NetWork/Ssh.h
Batch.o: Batch.c ../../Config.h ../Common/Global.h \
 ../Common/../Files/GabeditFileChooser.h \
 ../Common/../Common/GabeditType.h ../Utils/Utils.h \
 ../Utils/UtilsInterface.h ../Utils/Constants.h \
 ../Utils/GabeditTextEdit.h ../Common/Run.h ../Common/Windows.h \
 ../NetWork/Batch.h RemoteCommand.h
