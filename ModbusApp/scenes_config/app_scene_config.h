//---------------------------------------------------------------------
//  Here you gonna put all the scenes you wanna use, they have to be in
//  order.
//
//  For any scene you "add", you have to create 3 functions in every scene,
//  they can be on one file or can be in differents files but must be included
//  in the program
//
//  The name of the functions will be given in the app_scene_functions.c
//  there is a code that defines the names and the functions
//---------------------------------------------------------------------

ADD_SCENE(app, main, main)
ADD_SCENE(app, settings, settings)
ADD_SCENE(app, sniffer, sniffer)
ADD_SCENE(app, sender, sender)
ADD_SCENE(app, byte_input, byte_input)
ADD_SCENE(app, manual_sender, manual_sender)
ADD_SCENE(app, msgs_buffer, msgs_buffer)
