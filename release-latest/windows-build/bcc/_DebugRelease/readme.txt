-------- THE STEEM ENGINE --------

By Anthony and Russell Hayward

http://steem.atari.st/
E-mail: steem@gmx.net

Thank you for downloading the STE Emulating Engine, which from now on I will call Steem to save my precious typing fingers. Our aim is to make Steem the most accurate and easy to use emulator possible.

Steem is freeware, you don't have to pay anything or do anything to entitle you to use the program. However if you like it, or if you don't, you can e-mail us your thoughts: steem@gmx.net

We've tried to make Steem as straightforward as possible, so I will not go into great detail about anything. :-)

---- WHAT'S IT FOR? ----

An ST Emulator attempts to recreate the Atari ST computer in software on a PC. This means you can play your favourite ST games and even run applications without needing an ST, in fact it is often more pleasant using an emulator (none of those horrible mouse ports to fight with)! With Steem running you will have a window on your desktop that works just like an ST display.

---- GETTING STARTED ----

If you can't work out how to run Steem and get a ST program working then try following the getting started guide on the Steem website:

http://steem.atari.st/beginners.htm

---- TROUBLESHOOTING ----

If you have problems running Steem look in the faq.txt file, that covers the most common difficulties and what you can do about them. If you can't find the answer in there then check the website FAQ: 

http://steem.atari.st/faq.htm

It should be the same, but we might have updated it if many people are having the same problem. If your problem isn't covered then send an e-mail to steem@gmx.net with a full description and we'll try to sort it out if we can.

---- STARTING AND STOPPING ----

The main window contains the ST display and a toolbar. Click on the yellow play button with the left mouse button to start emulation. Immediately your mouse cursor will disappear, so you can control the ST cursor. To get your PC mouse back again press the Pause/Break key. Emulation will still continue while your mouse is running free, you can click anywhere on the ST display or press Pause/Break again to regain control of the ST mouse. To stop emulation either click the run button again or press Shift + Pause/Break. If you right click on this button then you can run Steem in slow motion, this is useful if you want to take a screenshot or see something that disappears very quickly.

If you are used to the first Win32 ST Emulator, WinSTon, you might want to use F12 to release the mouse - you can use one of Steem's features to allow this (see the shortcuts section below).

---- FAST FORWARD ----

Lots of ST games have tedious animated intro sequences that can last for minutes or even days. Press and hold the green fast-forward button and Steem will go full-steam ahead to get through it as soon as possible. If you right click on this button Steem will do a searchlight fast forward. Some programs can stop with a black screen, you have no idea whether they have crashed or are waiting for you to do something. Searchlight causes the ST screen to be displayed with different colours so you can see whatever is on it and work out what to do. If you double click on this button (with the left or right button) it  will stick down, so you don't have to hold down the button for particularly long waits.

---- RESET ST ----

The button to the right of fast forward resets the ST, a left mouse click causes a cold reset (the same as turning the ST off, waiting 30 seconds and then turning it on again) and a right click causes a warm reset (the same as pressing the reset button on the back of the ST). 

---- LOAD/SAVE MEMORY SNAPSHOT ----

The camera and chip button on the Steem toolbar brings up a menu from which you can load and save memory snapshots. Save snapshot will save out the current state of the ST to a file. This can be useful for games without save facilities. When you load a snapshot it will change your TOS version, monitor type, memory size and the current disks in the drives. The last ten  snapshots that have been used appear also on this menu so you can quickly reload them.

---- TAKE SCREENSHOT ----

Clicking on the camera and screen button will save an image of the current Steem display to your PC's hard drive or to the Windows clipboard. It is generally easier to assign this function to a key using Steem's shortcuts feature (see below). Right click on the button to see the options available, these can also be also be configured in the Options dialog (see below).

---- PASTE TEXT INTO ST ----

This button allows you to quickly and easily paste text from Windows into the ST. Just click on the button and Steem will type in the text at incredible speed. Some programs may struggle with the top speed, to slow it down right click on the button and choose a longer delay.

---- FULLSCREEN ----

The fullscreen mode is a bit tucked away on Steem. To go fullscreen you must click on the maximise box of the Steem Engine window. When in fullscreen mode the Pause/Break key stops emulation, you can't release the mouse and continue running. To go back to windowed mode, click on the far right button on the Steem toolbar. You need DirectX installed on your computer for fullscreen to be possible.


---- CONFIGURATION ----

The buttons on the right side of the window are for configuring the emulator.

---- THE DISK MANAGER ----

On the far right is the disk manager. Clicking on the button will make it appear, clicking again will make it disappear. This window controls all the disks on the ST, the 2 boxes up the top hold the disks currently in the drives. The large box at the bottom shows all the disk images in the current directory. The disk manager is like a little explorer window, you can go into a folder by double clicking on it or selecting it and pressing return. All disk image files, zip/rar archive files and any Windows shortcuts to those types of files are shown. 

.Disk Images
Disk images are floppy disks turned into files, this is how most programs are accessed on ST emulators. To put a disk in a drive, drag it to the box to the right of the big drive icon. To remove it from the drive just drag it back out to the directory view. If you right click in the directory view a menu will pop up allowing you to create a new standard size disk image, a custom size disk image or a folder. Custom disk images can be up to 2 megabytes in size, although larger ones are emulated correctly most TOS versions didn't support them, you may find they will only work with TOS 2.06 or maybe not at all.

.Hard Drives
The other important button on the disk manager is the Hard Drives button, it opens the hard drive manager. You can have up to 10 virtual hard drives, to set one up select the folder on your PC and then select the letter that you want it to be on the ST. Hard drives aren't as reliable as disk images, most things will work but not everything. If a program isn't working properly copy it to a disk image (using the ST desktop) to see if that helps. Another option in this box is to choose which hard drive to boot from if floppy drive A is empty. You can force Steem to boot from a hard drive even when drive A contains a disk by holding down the CONTROL key when you run after performing a cold reset of the ST.

.Home Folder
The disk manager allows you to select a home folder. This is where all your disks or shortcuts to disks should be stored. You can go to your home folder at any time by clicking the third button from the left in the disk manager window.  If you are away from your home folder you can quickly move/copy/create shortcuts to disks in your home folder by right mouse button dragging them to the home button. The button to the right of the home folder button sets the current folder as your home folder. Right clicking on the home button and the set-home button brings up menus allowing you to go to/set 10 quick-folder links.

To the right of the set-home button is a the disk manager options button, it brings up a menu with the following choices:

.Disconnect Drive B
By default Steem emulates an ST with an external drive attached, this is usually very useful. Unfortunately, most STs only had one drive and some programs will get confused when there are two. This option allows you to virtually yank out the external drive lead to fix those problems (it is usually a good idea to reset after you change this option as the ST might still think there is a drive B when really there isn't). Clicking on the big drive B icon can also toggle this option.

.Accurate Disk Access Times (Slow)
Floppy disk drives are slow, horribly tediously slow, so by default Steem does away with the waits and emulates disk access at impossibly high speed. Unfortunately, some ST software uses, ahem, colourful ways of accessing the STs floppy disk controller that only work if the disk access is horribly, tediously slow. So that is the reason for this option, but don't leave it on all the time or you will surely go insane!

.Read/Write Archives (Changes Lost On Eject)
To access disk images in archives (ZIP or RAR files) Steem extracts them to a temporary file and uses that like a normal disk image file. Unfortunately re-archiving the temporary file is very difficult, Steem can't do it. Because of this by default all disk images in archives are treated as read-only, so they can't be changed. By turning on this option you can make Steem allow writes to the temporary file, but any changes will get lost when the temporary file is deleted (the disk is ejected). Some ST programs require their disk to be read/write, this option will allow you to run them without having to extract them.

.Hide Broken Shortcuts
This option hides shortcuts which point to a file that doesn't exist. If you show these shortcuts then you can right click on them and select Fix Shortcut to allow Windows to search for the target file. You may want to check this if you have many broken shortcuts that are going to stay broken to speed up displaying of their folders.

.Eject Disks When Quit
This makes Steem eject all the disks from the drives when you quit.

.Open Current Folder in Explorer
Opens the folder the disk manager is looking at in Windows Explorer.

.Folders Pane in Explorer
When checked causes the folders pane to appear when you open a folder in Explorer.

.Find in Current Folder
Opens a Windows find dialog with the "Look in" directory set to the current folder, useful if you have a large number of disks in many folders and you want to find one quickly.

.Run MSA Converter
This loads the fantastic MSA Converter program that can be used to edit disk images.

.Import WinSTon Favourites
Clicking on this option brings up a dialog box that allows you to create shortcuts from WinSTon's favourites system. First select the folder containing WinSTon which has the favourites you want to import. Next select the folder that contains WinSTon's disk images (usually called discs). Now select a folder to import the favourites to. The option Only Downloaded Disks when checked causes Steem to only create shortcuts to disks that already exist, when unchecked Steem could end up creating thousands of broken shortcuts which will become unbroken if you download their target disk using WinSTon. The last option tells Steem what to do if the shortcut already exists at the required location, if you set this to skip then it would be quite quick to import the favourites again if you download a new disk through WinSTon you want to use in Steem (thus avoiding the bind of creating loads of broken shortcuts).

.Double Click On Disk...
This allows you to configure what happens when you double click the left mouse button on a disk image (or a shortcut to a disk image). By default doing this will insert the disk into drive A, reset the ST and then run.

.Close Disk Manager After Insert, Reset and Run
If you don't like the disk manager open when you run a game, give this a go.

.Large/Small Icons and Icon Spacing
Save space with small icons.


---- JOYSTICKS ----

The button to the left of the disk manager configures joysticks. Steem allows you to use any Windows compatible joystick in any way you want to control up to 8 ST Joysticks. 

The first option is what method to use for reading joysticks, Windows Multimedia should work fine for most joysticks but if you have problems (or if you have more than 2 joysticks) try DirectInput. 

The next option allows you to select one of 3 configurations for the ST joysticks, this is useful if you want to switch between different setups quickly.

Now it is time to configure the ST joysticks themselves, the important joysticks are the ones that went into the main ST ports (under the keyboard), but you can also set up ones that went in the STE only ports (next to the cartridge slot) and the rare parallel port joysticks (the ones that went where the printer did). You will see that there are 2 identical boxes, when "Standard Ports" is selected the left side configures the ST joystick that would have gone in the mouse port. The right side is for the joystick that would have gone in the joystick port. 

.Active
This option allows you to choose when a joystick is active. You may wonder why you would want to deactivate a joystick, but there is a good reason. When you use keys to control a joystick they become disabled for their normal purpose, the ST will no longer receive key presses. If you use cursor keys to control a joystick this can be very irritating as many games (and most other types of programs) use them to control menus or text input. If you select "Always" then the selected input will always control the joystick (and will not be passed on to the ST). If you choose "When Scroll Lock On" or "When Num Lock Off" then the joystick will only be active when that condition is met.

.Input Pickers
Steem uses a unique system for allowing a user to choose how they want to control the ST and the emulator. In the joystick dialog you will see light grey boxes that go white when you click on them, these are input pickers. When white you are able to press any key, move/press a control on any joystick (axis/POV/buttons) or press the middle mouse button (you must click on the input picker) to choose that as the input. There is one special key, Pause/Break that will make an input picker blank. Left and right shift (and left/right alt/control on Windows NT) will normally be treated as separate keys, if you want Steem to allow either key then press them both at the same time. If you want to use the tab key as input then you have to hold down control while pressing it (otherwise it switches the focus to the next control). For each joystick there is one input picker for left, right, up, down, fire and autofire. 

.Autofire
Autofire is turned on by selecting a speed, when it is on activating the selected input will act as if you were pressing and releasing the joystick's fire button very quickly again and again. This is handy for games that won't allow you to hold down the button to fire multiple shots.

.Deadzone
Clicking with the left mouse button in the middle box sets the dead-zone, this is the distance an analog joystick axis has to move before it counts as being pressed. 

.Mouse Speed
This is a useful option if you have a mouse that is designed to move around a 1600x1200 screen and it is moving on a 320x200 screen on Steem. I know this isn't technically a joystick configuration option, but it went in the same port!

.Oddities
It seems that some keyboards won't allow certain keys to be pressed at the same time. For example left cursor key, up cursor key and space. If you find the keyboard controlled joystick unresponsive try altering the key assignments, like making shift or control fire. There is also a problem with holding shift and pressing numpad 4, 6, 8 or 2 at the same time, Windows handles these combinations very poorly so it is best to avoid using them for anything.


---- OPTIONS ----

The button to the left of joysticks sets up loads of options.

o MACHINE

These options configure the hardware inside the virtual ST.

.ST CPU speed
This option can boost the speed of the emulated ST, to improve those painfully slow games that were released. The ST's CPU ran at 8 megahertz (8,000,000 clock cycles per second) but here you can whack it up to get things going faster. A couple of points: setting the CPU speed above 8MHz will cause the emulator to run _slower_, because it has more processing to do for each screen displayed. It will also cause some ST programs to crash/not work properly, because they rely on the processor running at the correct speed.

.Memory size
Here you can select how much memory will be available in the virtual ST. Please note that 14Mb wasn't supported on the ST range of computers without special hacks, so not everything will work with that option selected. Also 4Mb or greater does not work with TOS 1.00.

.Monitor type
Can be "Colour" (for ST low and medium resolutions) or "Monochrome" (for ST high resolution). Also available are extended monitors, these are larger screens than a standard ST could manage but are achieved by some tinkering with the ST operating system. The only programs that have any chance of working with extended monitors are GEM applications (windows/menu bars), all other programs will crash or make a mess of the screen (there is nothing that can be done about that). Most GEM apps will fail too, but some will work and allow you to fit more on the screen than anyone could ever need! NOTE: Extended monitors only work with TOS versions above 1.02, also if you use a 4 plane resolution (e.g. 800x600x4) the ST can get very confused if it tries to change to medium resolution, so try to avoid it. You have to do a cold reset of the ST before any changes to this option take effect.

.Keyboard
Here you can configure the keyboard, this is an area that many people have problems with because most non-English PC keyboards are vastly different from their ST counterparts. The important thing is that your PC keyboard language, TOS language and ST keyboard language all match, that gives you a chance of having the right characters appear for the right keys. If keyboard language is set to certain languages then the "Shift and alternate correction" option will become available. When checked this makes Steem do its best to get round the differences between ST and PC keyboard layouts by fooling the ST into thinking you are pressing keys you aren't. This works well for GEM programs but it could cause problems for games and any other program that reads the keyboard directly. If you can't get the keyboard to work properly then the only option is to manually remap the keys using shortcuts (see below).

.Cartridge
Steem can emulate ROM cartridges using cartridge images, here you can choose to insert one. For details how to make a cartridge image see the "cart image howto.txt" file that comes with Steem.

o TOS

Here you can choose what version of the ST operating system you want to use. Displayed in the box are all valid TOS images found in your Steem directory and also your home folder. You have to do a cold reset of the ST before any changes to this option take effect. 

.US TOS versions 
Using a United States TOS causes games to be run in the NTSC screen mode (60Hz). This is okay emulation-wise but a lot of ST games and demos do not work in this screen mode, and often just crash or act oddly rather than telling you why. We recommend that American users of Steem use UK TOSs for games and demos, the only time a US version is required is for word processing, due to tiny differences between the US and UK keyboards. 

.TOS 1.06 and 1.62 UK
Watch out, before Steem came along most copies of these TOSs were corrupt (because no emulator could test them properly). If you have problems with this version try downloading a new copy.

.TOS 1.00 
Is really bad, try to avoid using it unless a game insists on it (some really old ones do).

o PORTS

This page configures the emulated ST's link to outside hardware. There is a section for each port on the ST, MIDI, parallel and serial. To set up a port select one of the options from the box next to "Connect To". When you do the section will fill up with options related to the connection you chose. 

.MIDI Device
When you select this you must then select the PC MIDI device you want the ST's messages to be sent to and the PC MIDI device you want to be able to send data to the ST. 

.Parallel Port (LPT)
This allows you to connect an ST port to one of your PC's parallel ports. The only option is to choose which one. Warning: This may not work on all versions of Windows.

.COM Port
By selecting this option you can connect an ST port to one of your PC's COM ports. Again, just select the required port. Warning: This may not work on all versions of Windows.

.File
Using this option you can send all output from an ST port to a file. Click on the "Change File" button to open the file selector where you can select the output file (you can create a new one by typing in a new name and clicking "Open"). The "Reset Current File" button will delete everything that is currently in the file.

.Loopback (Output->Input)
Selecting this will cause anything that is sent to the port to be then received by the port, what use that is I really don't know!

o MIDI

Here are some options that affect the PC side of Steem's MIDI emulation.

.Volume
Not that useful but here never the less, it won't work for all devices.

.Allow running status for output/input
Normal MIDI messages are made up of one status byte that describes the message and then 1 or 2 data bytes. If a program wants to send the same message again it can leave off the status byte and just send the data. By default Steem doesn't allow this and adds the status byte itself if it has been omitted, this could cause problems to programs that require a very high transfer rate. These options mean that running status will be passed on to Windows which has to decide what to do with it, so if you are having problems with MIDI tempo you can try this.

.System Exclusive Buffers
The number of system exclusive buffers affect MIDI when it is sending/receiving many small system exclusive messages quickly. If the number of buffers is too small it is possible for there to be so little time between messages that Steem is still busy with all the buffers when a new sysex message arrives (and therefore part of it gets lost). If you find some MIDI program having input or output problems that sound like this it's worth a try increasing the number of buffers, something like 6 should do the job for almost anything. The size of the buffers allows you to choose the maximum length of message Steem can send or receive. This is important if you are transferring large memory banks from the device to the ST and vice-versa. 

.Input speed
This option allows you to slow down the speed Steem feeds any received data from the MIDI device to the ST. Due to restrictions of Windows Steem sends all data as fast as possible to the ST regardless of the speed it was actually sent by the device, some ST programs can't handle that. If you have problems with receiving bank dumps it might be an idea if you slow this down a bit. Also related to this is the shortcut action "Pause until next SysEx", this can get round time-outs on ST programs waiting for a bank dump to start. See the shortcuts section for details of how to assign this action to a key/combination of keys.

o MACROS

This is a handy feature that will allow you to record sequences of input in to the ST and play them back later. At the top there is a display of your macro folder, each macro is stored in a file with the extension stmac, they are displayed here (if you have any). To record a macro first click on "New Macro", name it something snazzy and then click on the big red record button. Now go to the main Steem window, start emulation and do some sort of input (move the mouse, press a key, move a joystick). When you have recorded what you want to go back to the macro display and click on the record button again to turn it off. Don't worry about doing this too quickly, Steem doesn't start recording until some input is received and it will cut off any time on the end that doesn't have any input. Now you can replay the actions you recorded by pressing the green play button (next to the record button). 

You can have as many macro files as space on your hard drive will allow, this might mean they get a bit difficult to find on the list, you can organise them into folders using the macro folder display. Just right click to create a new folder and drag appropriate macros in there. You can also right click on the bold "Macros" folder and choose to open it in Windows explorer if you prefer to use that for your organising.

There are 2 options that effect how macros work:

.Maximum mouse speed
This affects recording and playing macros, some ST programs won't accept really fast mouse movements as ST mice couldn't generally move very far in a short time. This option lets to allow for this problem by configuring the maximum speed for each macro individually.

.Playback event delay
For most macros you will want your actions to be played back as quickly as possible (for instance in the case of clicking on a shoot button in a game). This option allows you to configure how long Steem will wait between input changes, some programs won't accept lots of very quick inputs. If speed isn't an issue then you can always set this to "As Recorded", then playing the macro will replay the recorded events at the speed you recorded them at.

o GENERAL

.Run speed
Sometimes you will find a game/program that isn't significantly affected by changing CPU speed, it is still too slow. In that case you can use this option to change the speed Steem runs and make sure a program speeds up (or even slows down if you want). The disadvantage is that sound doesn't work above 105% or below 80%.

.Slow motion speed
This option determines how fast Steem will run when you have slow motion turned on (right-click on the play button).

.Maximum fast forward speed
Using this option you can limit Steem's speed, fastest isn't always best (you can find you have zipped through the bit you wanted to see).

.Show pop-up hints
This little option toggles those pop-up hints on and off, now you've read the readme you don't really need them.

.Make Steem high priority
When this option is ticked Steem will get first use of the CPU ahead of other applications, this means Steem will still run smoothly even if you start doing something in another window at the same time, but everything else will run slower.

.Pause emulation when inactive
When this option is checked Steem will pause emulation when you switch to another program. Steem uses a lot of PC CPU time when running which slows all other programs down, if you are switching between programs regularly you may want to use this option.

.Disable system keys when running
This option allows you to send certain key combinations that are used by Windows to the ST instead. When it is checked Alt+Tab, Ctrl+Esc and Ctrl+Alt+Delete will go to the ST instead of to the PC. Although Steem is very stable this option could cause a bit of a disaster is Steem stops responding for any reason, you won't be able to shut it down with Ctrl+Alt+Delete, for safety it is best to only use this option when you really need it. Unfortunately this option doesn't always work in fullscreen mode. Depending on your version of Windows, it may not be possible to capture certain key combinations - you can use shortcuts (see below) to trigger the keys you want from other keys. 

.Automatic fast forward on disk access
Sometimes disk access can be really slow, so this option makes Steem always fast forward when the ST is doing it. There are some programs that continue while accessing the disk, if you are going to use one of them then you should turn this off.

.Start emulation on mouse click
When checked clicking on Steem's main window will start emulation.

o SOUND

.Output type
This can be "None" to make no sound at all or "Simulated ST Speaker" to make sounds like the Yamaha 2149 sound chip that was in the ST coming through a standard speaker. "Direct" makes Steem emulate the sound chip in the way almost all other ST and Yamaha 2149 emulators do, this isn't how most STs would have sounded but some people might prefer it (it is also faster as it needs very little calculation). The last option is "Sharp STFM Samples", this makes Steem switch automatically to direct sound when a sample is being played (please note this doesn't work for all samples).

.Configuration
These options allow you to tweak the sound output, firstly volume, best to leave that around Max. Next is frequency, most sound cards can handle 50066Hz but if sound is a bit dodgy then you can go down to 44100Hz, if that doesn't work try 25533Hz (it will be quite muffled). The next option is format, 8-bit mono should be adequate for most people and it is the least stressful on the CPU. Then there is Write to primary buffer, some sound cards seem to perform very badly with Steem's standard output, try checking this if it doesn't sound perfect. Next is timing and delay, these options were added in a desperate attempt to improve output on some sound cards, if you are having problems fiddle and see if they make any difference.

.Record
These options allow you to record Steem's output to a wav file. Clicking on the red button begins recording, clicking on it again (or stopping emulation) will end recording. You can choose the name and location of the wav file to output by clicking on choose.

.Internal speaker sound
This is a bit of fun really, if you haven't got a sound card you can make your internal PC speaker output Steem's sound, try it with a few tunes!

o DISPLAY

.Frameskip
This is a very useful option for slower processors, drawing the screen takes quite a bit of PC CPU time so skipping frames is a very effective way to speed things up to the same speed as a real STE. Auto frameskip makes Steem skip up to 8 frames dependant on how fast it is running. 

.Borders
Here you can choose whether you want to see the ST borders. Some games use a technique called "overscan" to display graphics in the borders, that's one reason you might want to see them.  The option "Auto Borders" will keep them turned off most of the time, only coming on when there's an overscan.  You can also choose to have borders always on or always off.  The border option affects fullscreen mode: with borders on, Steem displays fullscreen in the 800x600 screen resolution.  Auto-borders won't work so well in fullscreen because we don't want to keep changing the PC monitor's resolution; so if you are running a program in fullscreen that makes use of the borders, turn on "Always Show Borders".

.Window size
These options allow you to configure exactly the size of Steem's window in the various ST resolutions. First is the option of whether Steem should automatically resize its window when the ST resolution changes or when borders turn on or off. Below that you can set the sizes that Steem should use. WARNING: On some video cards making the size bigger than the real size (the first option) will slow down emulation considerably.

When automatic resizing is off you may find it difficult to get the window to a precise size, making ugly lines appear; to help there are some options on the main window's system menu (accessed by clicking on Steem's icon in its title bar).  Normal size will resize the window to the size you have selected for the current ST screen resolution. Restore aspect ratio will maintain the current size of the window but will alter it so that its aspect ratio matches that of the selected size for the ST resolution. Also on this menu is Bigger and Smaller Window, they are a quick way to resize the window without going to the Options dialog, and border settings, which also affects the size of the window. As well as all that there's an "Always On Top" option that keeps the main Steem window above all others.

.Screenshots
These options configure where screenshots are saved, to take a screenshot set up a shortcut to perform the action "Take Screenshot". If you have the FreeImage library (http://freeimage.sourceforge.net/) then you will be able to choose the format that the screenshots will be saved in here. The last option is "Minimum size screenshots", when checked this will make Steem always save screenshots at the smallest size possible for the resolution, this is handy for people taking screenshots to put on a website. Warning: Sometimes this might make a mess as Steem will have to shrink the picture, this is never a problem if Fullscreen Mode->Drawing mode is set to "Stretch Blt".

o ON SCREEN DISPLAY

The Steem OSD is there to look pretty and give information, it can be configured to suit your needs.

.Floppy disk access light
This is a small light that appears in the top right of the screen, it emulates the light that was on the right hand side of a real ST.

.Logo
You can configure how long the Steem Engine logo will be displayed after you start emulation.

.Speed bar
The blue bar in the bottom-left corner indicates the current speed of the emulator compared to the refresh rate of the monitor on a real ST - if the bar is full it is drawing at the ST sync rate. 

.State icons
These little icons tell you whether the emulator is running, fast-forwarding, sound recording or just stopped.

.CPU speed indicator
This icon appears next to the speed bar when you have ST CPU Speed set to more than 8 megaherts.

.Scrolling messages
These messages pop up to delight and inform you, they have some useful tips for how to get the best out of Steem. 

.Disable on screen display
This will disable all elements of the on screen display, it is useful if you are taking screenshots.

o FULLSCREEN MODE

.Drawing mode
This is a bit of a confusing option, you really don't need to know what the possible settings mean, but which is best will be different depending on your computer's hardware, try them all to see which is quickest. Fullscreen drawing mode also affects windowed mode when the ST is in low res and the low res window is double size. Make sure you try all the modes, it can make a big speed difference.

.Special effects
When you are in "Straight Blt" or "Screen Flip" drawing mode you can apply effects to Steem's display. "Scanline Grille" makes every other vertical line black, this can make the display a bit dark but it can also make Steem run quicker. 

.Use 256 colour mode
When checked this makes Steem use the 8-bit graphics mode - this is faster but will not look good with very colourful games (for 8-bit mode the maximum is 118 colours on screen at any time, most ST programs only need 16). This option is very useful for high-res, because there are only 2 colours to display and using 65536 or more seems excessive.  

.Use 640x400
This option makes the screen switch to 640x400 pixels when you run in fullscreen. This means the ST display will fill the entire screen instead of having small borders to the top and bottom. You can only use 640x400 mode when borders are set to "Never Show Borders".

.Synchronisation
When the PC and ST are displaying a different number of frames per second you can get jerkiness and wobbling that didn't appear on a real ST. Using these options you can hopefully put that right, Vsync to PC display tells Steem to wait for the PC to be ready before it displays its next frame. To make this option work better you should try and make the PC refresh rate the same or double the ST one. The ST used 50Hz (PAL), 60Hz (NTSC) and 70Hz (Mono).

o BRIGHTNESS/CONTRAST

Some PC monitors/video cards can be very dark, this can make the ST display unrealistic, or even make some things that should be visible black. Here are some simple options to fix this problem, just fiddle with the two values until in the colour bar display you can see colours above the number 2 and the colours above 15 and 16 are a different shade. Sometimes other colours on the screen can make this difficult, to get round that click on the image of the bars to make it fill the whole screen.

o PROFILES

Profiles allow you to store many different Steem configurations and use them when you like. To make a new file containing the current Steem configuration click on "Save New Profile". You can then restore it at any time by selecting it and clicking on "Load Profile", be careful though as your configuration before you load a profile will be lost forever. Below the "Load Profile" and "Save Over Profile" is a list of sections that affect loading profiles, you can enable or disable each section. This allows you to only load specific options in Steem and keep other options that you don't want changed. As in macros (see above) you can organise all your profiles using the profile folder display.

o STARTUP

.Restore Previous State
You can choose to have Steem remember the state of the ST when it quits, so you can return to what you were doing next time you run it. Below the option is a box where you can type in the filename that will be used to save the state, this file will be saved in the directory Steem.exe is in.

.Start in Fullscreen Mode
If DirectDraw is enabled and is working okay then you can make Steem fill the whole screen when it first runs.

.Draw direct to video memory
To be able to display a picture on your monitor Steem has to get it to your video card's memory, it can do this in 2 ways. Firstly it can draw it direct to the video memory, it does that when this option is on. This is generally the fastest way to draw but on some set-ups it may cause problems if video card access is slow. With this option off Steem will draw to your PCs system memory, this is faster but at some point all the data will still have to be transferred to the video card. In older versions of Steem repeated accesses to video memory (to draw complex scanlines) could make it run about one fifth of the speed it should. Now when drawing direct to video memory Steem buffers complex scanlines to stop this problem, but you still may find it quicker to draw to system memory on your PC.

.Hide mouse pointer when blit
Some video cards can make a mess when the PC mouse pointer is over the Steem window, with this option on that will never happen as Steem hides the mouse before drawing to its window. This can however cause the PC mouse to flicker while Steem is running, if that is a problem try turning it off and see if your video card is one of the messy ones.

.Never Use DirectDraw/DirectSound
If you have DirectX errors when you boot up, or DirectX doesn't work properly, then you can tell Steem to not use it. Sound doesn't work without DirectSound and drawing is slower without DirectDraw.

.Sound Driver
This lets you configure which DirectSound driver Steem will use, Default is usually the best option.

o ICONS

You can customise Steem's look by altering its icons. On this page you will see a box containing all the icons, just left click on one to be able to select an ico file to replace it with. Right clicking on an icon restores it to its default, click on the "All Icons to Default" button to restore them all. You can also download icon schemes to change all or just some of Steem's icons with a single click on the "Load Icon Scheme" button, go to:

http://steem.atari.st/icons.htm

To see what is available.

o AUTO UPDATE

This configures Steem's auto update program, it checks for when new versions of Steem and new/updated patches are released. At the top are some statistics for how well auto update is working. Below are a few options to make it work better, or turn it off completely. When a new version of Steem is detected a button will appear on Steem's toolbar. When you click on it a box will appear with full details of the new version and the choice whether you want to download it now, later or not at all. Auto update will not work from behind a firewall, if you are behind one disable auto update and join the Steem update mailing list from the Steem website's download page.

o FILE ASSOCIATIONS

Windows programs love to associate and de-associate programs with file types. This dialog allows you to choose which files you want to associate with Steem. Just click the "Associate" button next to the desired files. Here you can also choose whether you want files to open in a new Steem window or the current Steem window when you double-click on them from Windows Explorer.


---- SHORTCUTS ----

In this dialog you can set keys, joystick buttons/axes and the middle mouse button to do various actions.

In the top left is a display of your shortcuts folder, this contains all your shortcut files (extension stcut). You can select which shortcuts files you want to use by clicking on the icon of the file, a green tick means they are active, a red cross means they are not active. If you have read about macros and profiles in the Options box then you'll know how to create and organise the files. To the right of the folder display is a large box which displays some instructions for people who haven't read the readme. 

Next in the dialog is the shortcuts display, this shows the shortcuts that are in the currently selected file. If you select the default shortcut file (there should be one in there) you can see how shortcuts work. For each shortcut there are 3 input pickers (see joysticks section for a description of how they work) separated by "+", this shows you what combination of input will trigger the action (i.e. A + B + C will only perform the action if you press the A, B and C keys simultaneously). Next there is "=" followed by a large box, this contains a list of all the possible actions. Most of these are straightforward but a few need more information (these are all followed by "->"):

.Press ST Key
When selected a new input picker will appear to the right of the action box and you can click on it to select the ST key that the selected combination of inputs will press. In this input picker F11 will count as the "(" key on the ST keypad, F12 will be keypad ")", Page Up is Help and Page Down is Undo. 

.Type ST Character
This is similar to the Press ST Key action but it allows you to choose any character that the ST can produce with the current TOS, this is handy if you have a strange keyboard layout and you can't get all the symbols you need.

.Play Macro
When selected a button will appear, clicking on it will bring up a new dialog in which you can select a macro from your macro folder.

The "Del" button deletes the shortcut, you could probably work that out for yourself.

.Add New/Copy
Below the shortcuts are 2 buttons that allow you to create more shortcuts, "Add New" creates a blank one and "Add Copy" creates one that is the same as the shortcut above the buttons. If you create more shortcuts than can fit in the box then a scrollbar will appear allowing you to see them all.


---- PATCHES ----

Steem runs a lot of ST software but some programs just won't work properly. Often when debugging them we find a way around the bug, without being able to solve it. Now you can run these programs by using Steem's patch system. Select the program you want to run on the left and follow the instructions.


---- GENERAL INFO ----

The button to the left of shortcuts tells you a few "interesting" facts about the emulator. Click on the tabs at the top to switch between pages.


---- TIPS AND OTHER MISCELLANEOUS STUFF ----

Here's a few hidden features/bits of information that you might not know about:

o Steem can save screenshots but it can be a bit difficult to find out how. You have to open the Shortcuts dialog, choose what input will cause one to be taken and set the action to "Take Screenshot".

o Right clicking on a file or folder in the disk manager can access some very useful features that can save lots of time.

o Dragging things to the home button on the disk manager's toolbar is a very quick way to move things around.

o The disk manager supports drag and drop of almost anything into it, you can move/copy/create shortcuts into the current directory or insert disks directly into the drives.

o Steem is not related to WinSTon in any way, as some people think - it was written from scratch by us. It would have been difficult for even us (masters of making programs worse) to transform the quite stable WinSTon v0.5 to the unstable and buggy Steem v1.0!

o It is very easy to transport Steem between different computers without losing all your settings. You need the files Steem.exe, steem.ini and your shortcuts folder to be able to run Steem with your configuration intact. All you need then is a TOS version that you will have to select the first time you run Steem on the new computer. You might find it handy to have unzipd32.dll too, then you can use zipped disk images.


---- COMMAND-LINE OPTIONS ----

Here are some useful command line options for Steem, the easiest way to run Steem with them is to make a shortcut to Steem.exe, right click on it and select Properties. In the text box labelled Target add the command line options on the end (after the " if there is one).

NODD, GDI - don't use DirectDraw.

NODS, NOSOUND - don't use DirectSound.

.st, .msa, .stt, .dim, .zip, .rar, .stz file - load disk & go.

.sts file - load snapshot & go.

.stc file - insert specified cartridge.

NONEW - means that if one of the above files is passed to Steem and a Steem window is already open it will be opened in the current Steem and not in a new window.

OPENNEW - means the opposite of NONEW, the files will always be opened in a new Steem window.

NOLPT - this removes the option to connect an ST port to your PC's LPT ports (that option can cause a crash on some versions of Windows).

NOCOM - this removes the option to connect an ST port to your PC's COM ports.

INI=[filename] - load [filename] instead of steem.ini.

TRANS=[filename] - use [filename] as translation file.

SOF=[freq] - stands for Sound Output Frequency. Output will be forced to [freq]Hz and all other factors will be ignored (unless setting the sound card output to [freq]Hz fails, in that case Steem will ignore this option and use its usual method of determining output frequency).

WINDOW - force Steem to boot in windowed mode.

FULLSCREEN - force Steem to boot in fullscreen mode.

DOUBLECHECKSHORTCUTS - this makes Steem's disk manager check shortcut files more thoroughly, some people have had problems when a shortcut's target file name is in a different case to the actual file. This option makes the disk manager slower to display folders with shortcuts in them.

SCLICK - this option makes Steem output a click when you run and stop instead of a bump.

NOPCJOYSTICKS - don't look for any PC joysticks, sometimes joystick drivers do some weird things!

OLDPORTIO - we changed how I/O to LPT and COM ports worked in v2.4, if you are using Win9x/ME then you can use this switch if it doesn't work any more.

SCREENSHOT[=path] - this tells the currently open Steem to take a screenshot. If no Steem is open then it won't do anything.

ALLOWREADOPEN - this command line option makes Steem open hard drive files as read only. TOS has a bug in it that allows programs that open files for read to write to them, most versions of Windows and all versions of Linux don't allow it. Steem gets round this by opening all files as read and write. This works well except in circumstances when another Windows program is accessing the same file. If you want to do that then use this command line option to open the file as read-only.

STFMBORDER - since version 3 Steem has tightened up its border timings to make them more like a real STE, this has stopped some demos that use overscans from working properly. This switch makes Steem emulate border timings like an STFM, allowing you to view the screens.

SCREENSHOTUSEFULLNAME - this makes Steem use the full disk name as the screenshot name, rather than just the first word.


That's it! You now know everything you need to know about The Steem Engine. We hope you have fun using it.

---- CONTACT ----

We are very interested in people's comments about Steem. If you have any or you have ideas for how we can improve it please e-mail us at steem@gmx.net.

---- EMULATION BUG REPORTS ----

If you find an emulation bug please send an e-mail to: 

steembugs@gmx.net

Please include the name of the program with the bug and preferably where we can download it. Also the TOS image version number/country name, the memory size and your ST monitor settings when the problem occurs. Please don't send any big attachments (anything over 200Kb) without asking us first.

---- STEEM CRASH REPORTS ----

If Steem crashes or freezes up on you then we want to hear! It is very important to us that Steem is stable. Please send us an e-mail with as much info as you can, tell us what you were doing when it crashed/froze, if it was a one-off or a persistent error, and anything else you think might be appropriate:

steem@gmx.net

NOTE: You don't need to include all the technical information that appears in the Windows crash box, the name of the error might be useful but the rest doesn't help at all.


Thanks for reading this file, or just scrolling to the bottom, I hope you enjoy using Steem. Remember to check http://steem.atari.org/ regularly, updates are coming thick and fast! If you don't want to check too often then you can join the Steem update announcement list to get an e-mail whenever a new version is released, go to the Downloads page of the Steem web site for details of how to join.

Readme written by Russell Hayward

---- LEGAL STUFF ----

THIS PROGRAM AND DOCUMENTATION ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. BY USING THE PROGRAM YOU AGREE TO BEAR ALL RISKS AND LIABILITIES ARISING FROM THE USE OF THE PROGRAM AND DOCUMENTATION AND THE INFORMATION PROVIDED BY THE PROGRAM AND THE DOCUMENTATION.

