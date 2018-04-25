/*
 * $RCSfile: p3rexx.c,v $
 *
 * AProf Interface f�r Rexx Unmangler
 *
 * >> Modul kann mit Aztec und Maxon �bersetzt werden <<
 *
 * $Revision: 1.1.1.1 $ $Date: 2002/05/31 23:15:47 $
 *
 * � 1993,94 Michael G. Binz
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rexx/rexxio.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <rexx/errors.h>
#include <clib/exec_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <pragma/exec_lib.h>
#include <pragma/rexxsyslib_lib.h>

#include "pro.h"



/*
 * RexxSysBase mu� bereits ge�ffnet sein
 */

// Suffix f�r Rexx Unmangler
#define SUFFIX      "aprof"

// Puffer
static char rxBuffer[256];

// Zeiger auf den Namen des Rexx Unmanglers
static UBYTE *rxFunc;

// Reply Port
static struct MsgPort *rxReply;




/*
 * Freigeben aller Resourcen
 *
 * Nur _ein_ Aufruf bei Programmende!
 */
static void RexxExit( void )
{
   if ( rxReply )
      DeleteMsgPort( rxReply );

   if ( rxFunc )
      DeleteArgstring( rxFunc );
}




/*
 * Initialisierung (Freigabe erfolgt automatisch)
 */
BOOL RexxInit( void )
{
   atexit( RexxExit );

   if ( rxReply = CreateMsgPort() )
      return TRUE;
   else
      return FALSE;
}



/*
 * Sendet eine Message an den Rexx Host Prozess
 *
 * R�ckgabe: FALSE = Rexx Port nicht gefunden
 *           TRUE    Sonst
 */
static BOOL SendRexxMsg( struct RexxMsg *rm )
{
   BOOL ret = TRUE;
   struct MsgPort *targetP;

   Forbid();

   if ( targetP = FindPort( "REXX" ) )
      PutMsg( targetP, (struct Message *)rm );
   else
      ret = FALSE;

   Permit();

   return ret;
}



/*
 * Zeigt einen Laufzeitfehler von Rexx an
 *
 * Verwendung der Rexx-Funktion ErrorText()
 */
static void PrintRexxError( LONG err1, LONG err2 )
{
   // Rexx Message erstellen
   struct RexxMsg *rm;
   // �bergabe
   UBYTE *arg0;

   if ( rm  = CreateRexxMsg( rxReply, NULL, __FILE__ ) )
   {
      sprintf( rxBuffer, "exit errortext(%d)", err2 );

      if ( arg0 = CreateArgstring( rxBuffer, strlen( rxBuffer ) ) )
      {
         rm->rm_Action = RXFUNC | RXFF_STRING | RXFF_RESULT;
         ARG0( rm ) = arg0;

         SendRexxMsg( rm );

         if ( WaitPort( rxReply ) )
         {
            // R�ckgabe in Empfang nehmen
            struct RexxMsg *returned = (struct RexxMsg *)GetMsg( rxReply );

            // Auswertung der Ergebnisse
            if ( rm->rm_Result1 )
               p3err( "ARexx internal error" );
            else
            {
               // Ergebnis �bernehmen
               p3err( "ARexx error %d/%d: %s", err2, err1, (char *)rm->rm_Result2 );
               // �bergabestruktur freigeben
               DeleteArgstring( (UBYTE *)rm->rm_Result2 );
            }
         }

         DeleteArgstring( arg0 );
      }

      DeleteRexxMsg( rm );
   }
   else
      p3err( "ARexx error %d/%d", err1, err2 );
}



/*
 * Diese Funktion stellt f�r AProf den Unmangler dar
 *
 * symbol:   Zeiger auf ein symbol in C++ (o.�.) Notation
 *
 * R�ckgabe: Verwurschtetes symbol
 */
char *RexxUnmangle( char *symbol )
{
   // Initialisierung der Messagestruktur
   struct RexxMsg *rm = CreateRexxMsg( rxReply, SUFFIX, __FILE__ );

   if ( rm )
   {
      // �bergabewert
      UBYTE *arg1 = CreateArgstring( symbol, strlen( symbol ) );

      // Actionscode, Ergebnisanforderung, Argumentenzahl
      rm->rm_Action = RXFUNC | RXFF_RESULT | 1; // 1 Argument

      // Argumente in Message einh�ngen
      ARG0( rm ) = rxFunc;
      ARG1( rm ) = arg1;

      // Message senden
      SendRexxMsg( rm );

      // Auf Antwort warten (Leider ist w�hrend dem Warten nichts zu tun)
      if ( WaitPort( rxReply ) )
      {
         // R�ckgabe in Empfang nehmen
         struct RexxMsg *returned = (struct RexxMsg *)GetMsg( rxReply );

         // Auswertung der Ergebnisse
         if ( rm->rm_Result1 )
         {
            // Zur�ckschalten auf Standard Unmangler
            SetRexxFunc( NULL );
            // Fehler ausgeben
            PrintRexxError( rm->rm_Result1, rm->rm_Result2 );
            // Symbol unver�ndert zur�ckgeben
            strcpy( rxBuffer, symbol );
         }
         else
         {
            // Ergebnis �bernehmen
            strcpy( rxBuffer, (char *)rm->rm_Result2 );
            // �bergabestruktur freigeben
            DeleteArgstring( (UBYTE *)rm->rm_Result2 );
         }
      }

      // Argumente freigeben
      DeleteArgstring( arg1 );

      // Messagestruktur freigeben
      DeleteRexxMsg( rm );
   }

   // Ergebnis zur�ckgeben
   return *rxBuffer ? rxBuffer : NULL;
}




/*
 * Setzen eines Rexx Funktionsnamens
 */
void SetRexxFunc( UBYTE *name )
{
   if ( rxFunc )
      DeleteArgstring( rxFunc );

   if ( name && *name )
      rxFunc = CreateArgstring( name, strlen( (char *)name ) );
   else
      rxFunc = NULL;
}




/*
 * Lesen eines Rexx Funktionsnamens
 */
UBYTE *GetRexxFunc( void )
{
   return rxFunc;
}



/*
 * R�ckgabe des f�r Rexx reservierten ToolTypes
 */
char *GetRexxToolType( void )
{
   return "RXFUNC";
}
