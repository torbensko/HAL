/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
In a gist, you are free to use however you see fit, just please remember
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#ifndef HAL_TUNABLE_VAR
#define HAL_TUNABLE_VAR

/*
 * A value that either we or the user may wish to tune in order to 
 * perfect the way in which the HAL technique responds.
 */
class TunableVar
{
public:
	virtual void			SetValue( const char *value ) = 0;
	virtual void			SetValue( float value ) = 0;
	virtual void			SetValue( int value ) = 0;

	virtual void			Revert( void ) = 0;

	virtual float			GetFloat( void ) = 0;
	virtual int				GetInt( void ) = 0;
	virtual bool			GetBool() const = 0;
	virtual char const*		GetString( void ) = 0;
};


#endif
