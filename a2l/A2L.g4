grammar A2L;

@members {
    public static void main(String[] args) throws Exception {
        A2LLexer lex = new A2LLexer(new ANTLRFileStream(args[0]));
        CommonTokenStream tokens = new CommonTokenStream(lex);
 
        A2LParser parser = new A2LParser(tokens);
 
        try {
            parser.a2l();
        } catch (RecognitionException e)  {
            e.printStackTrace();
        }
    }
}
/*------------------------------------------------------------------
 * PARSER RULES
 *------------------------------------------------------------------*/
a2l
	:	version
		project
	;
version
	:	'ASAP2_VERSION' INT INT
	;
project
	:	'/begin PROJECT' ID STRING
		header 
		module+
		'/end PROJECT'
	;
header
	:	'/begin HEADER' ID? STRING
	 	'VERSION' STRING 
	 	'PROJECT_NO' ID 
	 	'/end HEADER'
	;
module
	: 	'/begin MODULE' ID? STRING
		(a2ml)*
		'/end MODULE'
	;
a2ml
	:	'/begin A2ML'
		CHAR*
		'/end A2ML'
	;
/*------------------------------------------------------------------
 * LEXER RULES
 *------------------------------------------------------------------*/
ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
    ;

INT :	'0'..'9'+
    ;

FLOAT
    :   ('0'..'9')+ '.' ('0'..'9')* EXPONENT?
    |   '.' ('0'..'9')+ EXPONENT?
    |   ('0'..'9')+ EXPONENT
    ;

COMMENT
    :   ('//' ~('\n'|'\r')* '\r'? '\n'
    |   '/*' ( . )*? '*/') -> skip
    ;

WS  :   ( ' '
        | '\t'
        | '\r'
        | '\n'
        ) -> skip
    ;

STRING
    :  '"' ( ESC_SEQ | ~('\\'|'"') )* '"'
    ;

CHAR:  '\'' ( ESC_SEQ | ~('\''|'\\') ) '\''
    ;

fragment
EXPONENT : ('e'|'E') ('+'|'-')? ('0'..'9')+ ;

fragment
HEX_DIGIT : ('0'..'9'|'a'..'f'|'A'..'F') ;

fragment
ESC_SEQ
    :   '\\' ('b'|'t'|'n'|'f'|'r'|'\"'|'\''|'\\')
    |   UNICODE_ESC
    |   OCTAL_ESC
    ;

fragment
OCTAL_ESC
    :   '\\' ('0'..'3') ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7') ('0'..'7')
    |   '\\' ('0'..'7')
    ;

fragment
UNICODE_ESC
    :   '\\' 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
    ;


