#!/usr/bin/env python3
"""PTY integration tests against the real application (optionally through elksemu)."""
import os,pty,fcntl,termios,struct,subprocess,select,time,tempfile,sys,signal,re
from pathlib import Path
command=[str(Path(x).resolve()) for x in sys.argv[1:]]
class UI:
 def __init__(self,left,right,env=None,controlling=True,rows=25,cols=80,probe_reply=None):
  self.master,self.slave=pty.openpty()
  fcntl.ioctl(self.slave,termios.TIOCSWINSZ,struct.pack('HHHH',rows,cols,0,0))
  self.saved=termios.tcgetattr(self.slave)
  def session():
   os.setsid()
   if controlling:fcntl.ioctl(0,termios.TIOCSCTTY,0)
  self.p=subprocess.Popen([sys.executable,str(Path(__file__).with_name('pty_child.py'))]+command+[str(left),str(right)],stdin=self.slave,stdout=self.slave,stderr=self.slave,preexec_fn=session,env={**os.environ,**(env or {})})
  self.data=b'';self.mark=0;self.probe_reply=probe_reply;self.probes=0
 def wait(self,text,timeout=6):
  text=text.encode();end=time.monotonic()+timeout
  while time.monotonic()<end:
   if text in self.data[self.mark:]:
    self.mark=self.data.index(text,self.mark)+len(text);return
   if select.select([self.master],[],[],.05)[0]:
    try:
     self.data+=os.read(self.master,65536)
     probes=self.data.count(b'\x1b[6n')
     if probes>self.probes:
      self.probes=probes
      if self.probe_reply is not None:os.write(self.master,self.probe_reply)
    except OSError:break
  raise AssertionError((text,self.p.poll(),self.data[-2000:]))
 def send(self,s):
  self.mark=len(self.data);os.write(self.master,s.encode())
 def finish(self):
  try:
   end=time.monotonic()+4
   while self.p.poll() is None and time.monotonic()<end:
    if select.select([self.master],[],[],.05)[0]:self.data+=os.read(self.master,65536)
   self.p.wait(timeout=.1)
   assert self.p.returncode==0,(self.p.returncode,self.data[-2500:])
   # The session-holder asserts restoration before the PTY is hung up.
  finally:
   if self.p.poll() is None:self.p.kill();self.p.wait()
   os.close(self.master);os.close(self.slave)
def cells(data,rows=25,cols=80):
 """Replay cmdr's ANSI output so delta redraws can be compared with a repaint."""
 grid=[[(' ',False) for _ in range(cols)] for _ in range(rows)]
 row=col=0;rev=False;saved=(0,0)
 for token in re.findall(rb'\x1b\[[0-9;?]*[A-Za-z]|\x1b[78]|[^\x1b]+',data):
  if token==b'\x1b7':saved=(row,col)
  elif token==b'\x1b8':row,col=saved
  elif token.startswith(b'\x1b['):
   params=token[2:-1];end=token[-1:]
   if end==b'H':
    parts=params.split(b';');row=min(rows-1,int(parts[0] or b'1')-1)
    col=min(cols-1,int(parts[1] or b'1')-1) if len(parts)>1 else 0
   elif end==b'J' and params==b'2':grid=[[(' ',False) for _ in range(cols)] for _ in range(rows)]
   elif end==b'm':
    for n in params.split(b';'):
     if n in (b'',b'0'):rev=False
     elif n==b'7':rev=True
  else:
   for ch in token:
    if ch==13:col=0
    elif ch==10:row=min(rows-1,row+1)
    elif ch>=32:
     grid[row][col]=(chr(ch),rev);col=min(cols-1,col+1)
 return grid

with tempfile.TemporaryDirectory(prefix='cmdr-ui-') as d:
 base=Path(d);left=base/'left';right=base/'right';left.mkdir();right.mkdir()
 content=('UNIQUE VIEWER TEXT\n'+'line\n'*50).encode();(left/'alpha.txt').write_bytes(content)
 ui=UI(left,right)
 try:
  ui.wait('Ready.');ui.send('\x1b[B');ui.wait('F10/Q Quit')
  ui.send('3');ui.wait('UNIQUE VIEWER TEXT');ui.send(' ');ui.wait('Page 2');ui.send('q');ui.wait('F10/Q Quit')
  ui.send('c');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?');ui.send('n');ui.wait('F10/Q Quit');assert not (right/'alpha.txt').exists()
  ui.send('\x1b[15~');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?');ui.send('y');ui.wait('File copied.');assert (right/'alpha.txt').read_bytes()==content
  ui.send('c');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?');ui.send('y');ui.wait('Copy failed');assert (right/'alpha.txt').read_bytes()==content
  ui.send('6');ui.wait('destination filename');ui.send('\x7f'*9+'moved.txt\r');ui.wait('Move file');ui.send('y');ui.wait('File moved.');assert not (left/'alpha.txt').exists() and (right/'moved.txt').read_bytes()==content
  ui.send('n');ui.wait('Create folder');ui.send('newdir\r');ui.wait('Folder created.');assert (left/'newdir').is_dir()
  ui.send('\x1b[F');ui.wait('F10/Q Quit');ui.send('\r');ui.wait('/newdir');ui.send('\x7f');ui.wait('F10/Q Quit')
  ui.send('\x1b[F');ui.wait('F10/Q Quit');ui.send('d');ui.wait('Permanently delete');ui.send('n');ui.wait('F10/Q Quit');assert (left/'newdir').exists()
  ui.send('8');ui.wait('Permanently delete');ui.send('y');ui.wait('Deleted.');assert not (left/'newdir').exists()
  ui.send('\t');ui.wait('F10/Q Quit');ui.send('\x1b[F');ui.wait('F10/Q Quit')
  ui.send('p');ui.wait('Directory path');ui.send('\x7f'*len(str(right))+'/nonexistent-cmdr\r');ui.wait('Directory:');assert (right/'moved.txt').exists()
  ui.send('?');ui.wait('keyboard guide');ui.send(' ');ui.wait('F10/Q Quit')
  ui.send('q');ui.finish()
 except BaseException:
  if ui.p.poll() is None:ui.p.kill();ui.p.wait()
  raise
 # Inherited terminal with no controlling /dev/tty, as on the PPC Telnet.
 for i in range(600):
  (left/('f%03d'%i)).touch();(right/('g%03d'%i)).touch()
 ui=UI(left,right,controlling=False)
 ui.wait('Ready.');ui.send('\x1b[F');ui.wait('601/601 entries')
 ui.send('r');ui.wait('601/601 entries')
 ui.send('c');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?')
 ui.send('y');ui.wait('File copied.');assert (right/'f599').exists()
 ui.send('q');ui.finish()
 # Signal-driven cleanup during input.
 ui=UI(left,right);ui.wait('Ready.');ui.p.send_signal(signal.SIGTERM);ui.finish()
 # Large remote terminal, number shortcuts, fragmented function keys, bare Escape.
 ui=UI(left,right,rows=40,cols=120,probe_reply=b'\x1b[40;120R')
 ui.wait('Ready.');ui.wait('F10/Q Quit')
 assert b'\x1b[40;1H' in ui.data and b'\x1b[2;61H' in ui.data
 ui.send('1');ui.wait('keyboard guide');ui.send(' ');ui.wait('F10/Q Quit')
 for seq in ['\x1bOP','\x1b[11~','\x1b[[A','\x1b[1P']:
  for ch in seq:ui.send(ch);time.sleep(.025)
  ui.wait('keyboard guide');ui.send(' ');ui.wait('F10/Q Quit')
 ui.send('2');ui.wait('Directory path');ui.send('\x1b');ui.wait('F10/Q Quit')
 ui.send('7');ui.wait('Create folder');ui.send('123\r');ui.wait('Folder created.')
 assert (left/'123').is_dir() # digits stay literal inside prompts
 if len(command)==1: # host shell; emulator has no ELKS /bin/sh fixture
  ui.send('9');ui.wait('# ' if os.geteuid()==0 else '$ ')
  ui.send('echo CMDR_SHELL_OK\r');ui.wait('CMDR_SHELL_OK')
  ui.send('exit\r');ui.wait('F10/Q Quit')
 # Bracketed paste must not execute a batch of commands.
 ui.send('\x1b[200~078\x1b[201~1');ui.wait('keyboard guide')
 ui.send(' ');ui.wait('F10/Q Quit')
 ui.probe_reply=b'\x1b[30;100R'
 fcntl.ioctl(ui.slave,termios.TIOCSWINSZ,struct.pack('HHHH',30,100,0,0))
 ui.send('r');ui.wait('\x1b[30;1H');ui.wait('F10/Q Quit')
 ui.send('0');ui.finish()
 # No window ioctl result: DSR response, timeout, and typeahead preservation.
 for response,expected in [(b'\x1b[35;100R',35),(None,25),(b'1',25),(b'\x1b[999999999999999999999;80R',25),(b'\x1b[35;;100R',25)]:
  ui=UI(left,right,rows=0,cols=0,probe_reply=response,controlling=False)
  ui.wait('Ready.')
  if response==b'1':
   ui.wait('keyboard guide');ui.send(' ');ui.wait('F10/Q Quit')
  else:ui.wait('F10/Q Quit')
  assert ('\x1b[%d;1H'%expected).encode() in ui.data
  ui.send('0');ui.finish()
 # Incremental painting must match an explicit full refresh, including highlights.
 ui=UI(left,right,probe_reply=b'\x1b[25;80R');ui.wait('F10/Q Quit');ui.wait('\x1b[0m')
 for keypress in ['\x1b[B','\x1b[B','\t','\x1b[B','\t','\x1b[F','\x1b[A','\x1b[H','\x1b[6~','\x1b[5~']:
  start=len(ui.data);ui.send(keypress);ui.wait('F10/Q Quit');ui.wait('\x1b[0m')
  delta=len(ui.data)-start
  if keypress=='\x1b[B':assert delta<600,delta
  incremental=cells(ui.data)
  ui.send('r');ui.wait('F10/Q Quit');ui.wait('\x1b[0m')
  assert cells(ui.data)==incremental,('delta repaint differs from full refresh',repr(keypress),[(r,c,incremental[r][c],cells(ui.data)[r][c]) for r in range(25) for c in range(80) if incremental[r][c]!=cells(ui.data)[r][c]][:15])
 ui.send('0');ui.finish()
print('PASS: incremental rows/highlights match full refresh; within-page navigation under 600 output bytes')
print('PASS: UI file operations, keyboard sequences and number shortcuts, bare Escape, 120x40 layout, resize/DSR/timeout/typeahead, paste suppression and terminal restoration (shell return on host only)')
