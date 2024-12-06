
#define LIBBLACKSQUID_IMPLEMENTATION
#include "../libblacksquid.h"
#include <stdlib.h>
#include <stdio.h>

const char *STOP_WORD_LIST[] = {
    "A", "ABOUT", "ABOVE", "AFTER", "AGAIN", "AGAINST", "AIN", "ALL",
    "AM", "AN", "AND", "ANY", "ARE", "AREN", "AREN'T", "AS", "AT", "BE",
    "BECAUSE", "BEEN", "BEFORE", "BEING", "BELOW", "BETWEEN", "BOTH", "BUT",
    "BY", "CAN", "COULDN", "COULDN'T", "D", "DID", "DIDN", "DIDN'T", "DO",
    "DOES", "DOESN", "DOESN'T", "DOING", "DON", "DON'T", "DOWN", "DURING",
    "EACH", "FEW", "FOR", "FROM", "FURTHER", "HAD", "HADN", "HADN'T", "HAS",
    "HASN", "HASN'T", "HAVE", "HAVEN", "HAVEN'T", "HAVING", "HE", "HER", "HERE",
    "HERS", "HERSELF", "HIM", "HIMSELF", "HIS", "HOW", "I", "IF", "IN", "INTO",
    "IS", "ISN", "ISN'T", "IT", "IT'S", "ITS", "ITSELF", "JUST", "LL", "M", "MA",
    "ME", "MIGHTN", "MIGHTN'T", "MORE", "MOST", "MUSTN", "MUSTN'T", "MY", "MYSELF",
    "NEEDN", "NEEDN'T", "NO", "NOR", "NOT", "NOW", "O", "OF", "OFF", "ON", "ONCE",
    "ONLY", "OR", "OTHER", "OUR", "OURS", "OURSELVES", "OUT", "OVER", "OWN", "RE",
    "S", "SAME", "SHAN", "SHAN'T", "SHE", "SHE'S", "SHOULD", "SHOULD'VE", "SHOULDN",
    "SHOULDN'T", "SO", "SOME", "SUCH", "T", "THAN", "THAT", "THAT'LL", "THE", "THEIR",
    "THEIRS", "THEM", "THEMSELVES", "THEN", "THERE", "THESE", "THEY", "THIS", "THOSE",
    "THROUGH", "TO", "TOO", "UNDER", "UNTIL", "UP", "VE", "VERY", "WAS", "WASN",
    "WASN'T", "WE", "WERE", "WEREN", "WEREN'T", "WHAT", "WHEN", "WHERE", "WHICH",
    "WHILE", "WHO", "WHOM", "WHY", "WILL", "WITH", "WON", "WON'T", "WOULDN", "WOULDN'T",
    "Y", "YOU", "YOU'D", "YOU'LL", "YOU'RE", "YOU'VE", "YOUR", "YOURS", "YOURSELF",
    "YOURSELVES", "COULD", "HE'D", "HE'LL", "HE'S", "HERE'S", "HOW'S", "I'D", "I'LL",
    "I'M", "I'VE", "LET'S", "OUGHT", "SHE'D", "SHE'LL", "THAT'S", "THERE'S", "THEY'D",
    "THEY'LL", "THEY'RE", "THEY'VE", "WE'D", "WE'LL", "WE'RE", "WE'VE", "WHAT'S",
    "WHEN'S", "WHERE'S", "WHO'S", "WHY'S", "WOULD", "ABLE", "ABST", "ACCORDANCE",
    "ACCORDING", "ACCORDINGLY", "ACROSS", "ACT", "ACTUALLY", "ADDED", "ADJ", "AFFECTED",
    "AFFECTING", "AFFECTS", "AFTERWARDS", "AH", "ALMOST", "ALONE", "ALONG", "ALREADY",
    "ALSO", "ALTHOUGH", "ALWAYS", "AMONG", "AMONGST", "ANNOUNCE", "ANOTHER", "ANYBODY",
    "ANYHOW", "ANYMORE", "ANYONE", "ANYTHING", "ANYWAY", "ANYWAYS", "ANYWHERE",
    "APPARENTLY", "APPROXIMATELY", "ARENT", "ARISE", "AROUND", "ASIDE", "ASK", "ASKING",
    "AUTH", "AVAILABLE", "AWAY", "AWFULLY", "B", "BACK", "BECAME", "BECOME", "BECOMES",
    "BECOMING", "BEFOREHAND", "BEGIN", "BEGINNING", "BEGINNINGS", "BEGINS", "BEHIND",
    "BELIEVE", "BESIDE", "BESIDES", "BEYOND", "BIOL", "BRIEF", "BRIEFLY", "C", "CA",
    "CAME", "CANNOT", "CAN'T", "CAUSE", "CAUSES", "CERTAIN", "CERTAINLY", "CO", "COM",
    "COME", "COMES", "CONTAIN", "CONTAINING", "CONTAINS", "COULDNT", "DATE", "DIFFERENT",
    "DONE", "DOWNWARDS", "DUE", "E", "ED", "EDU", "EFFECT", "EG", "EIGHT", "EIGHTY",
    "EITHER", "ELSE", "ELSEWHERE", "END", "ENDING", "ENOUGH", "ESPECIALLY", "ET", "ETC",
    "EVEN", "EVER", "EVERY", "EVERYBODY", "EVERYONE", "EVERYTHING", "EVERYWHERE", "EX",
    "EXCEPT", "F", "FAR", "FF", "FIFTH", "FIRST", "FIVE", "FIX", "FOLLOWED", "FOLLOWING",
    "FOLLOWS", "FORMER", "FORMERLY", "FORTH", "FOUND", "FOUR", "FURTHERMORE", "G", "GAVE",
    "GET", "GETS", "GETTING", "GIVE", "GIVEN", "GIVES", "GIVING", "GO", "GOES", "GONE",
    "GOT", "GOTTEN", "H", "HAPPENS", "HARDLY", "HED", "HENCE", "HEREAFTER", "HEREBY",
    "HEREIN", "HERES", "HEREUPON", "HES", "HI", "HID", "HITHER", "HOME", "HOWBEIT",
    "HOWEVER", "HUNDRED", "ID", "IE", "IM", "IMMEDIATE", "IMMEDIATELY", "IMPORTANCE",
    "IMPORTANT", "INC", "INDEED", "INDEX", "INFORMATION", "INSTEAD", "INVENTION", "INWARD",
    "ITD", "IT'LL", "J", "K", "KEEP", "KEEPS", "KEPT", "KG", "KM", "KNOW", "KNOWN",
    "KNOWS", "L", "LARGELY", "LAST", "LATELY", "LATER", "LATTER", "LATTERLY", "LEAST",
    "LESS", "LEST", "LET", "LETS", "LIKE", "LIKED", "LIKELY", "LINE", "LITTLE", "'LL",
    "LOOK", "LOOKING", "LOOKS", "LTD", "MADE", "MAINLY", "MAKE", "MAKES", "MANY", "MAY",
    "MAYBE", "MEAN", "MEANS", "MEANTIME", "MEANWHILE", "MERELY", "MG", "MIGHT", "MILLION",
    "MISS", "ML", "MOREOVER", "MOSTLY", "MR", "MRS", "MUCH", "MUG", "MUST", "N", "NA",
    "NAME", "NAMELY", "NAY", "ND", "NEAR", "NEARLY", "NECESSARILY", "NECESSARY", "NEED",
    "NEEDS", "NEITHER", "NEVER", "NEVERTHELESS", "NEW", "NEXT", "NINE", "NINETY", "NOBODY",
    "NON", "NONE", "NONETHELESS", "NOONE", "NORMALLY", "NOS", "NOTED", "NOTHING",
    "NOWHERE", "OBTAIN", "OBTAINED", "OBVIOUSLY", "OFTEN", "OH", "OK", "OKAY", "OLD",
    "OMITTED", "ONE", "ONES", "ONTO", "ORD", "OTHERS", "OTHERWISE", "OUTSIDE", "OVERALL",
    "OWING", "P", "PAGE", "PAGES", "PART", "PARTICULAR", "PARTICULARLY", "PAST", "PER",
    "PERHAPS", "PLACED", "PLEASE", "PLUS", "POORLY", "POSSIBLE", "POSSIBLY", "POTENTIALLY",
    "PP", "PREDOMINANTLY", "PRESENT", "PREVIOUSLY", "PRIMARILY", "PROBABLY", "PROMPTLY",
    "PROUD", "PROVIDES", "PUT", "Q", "QUE", "QUICKLY", "QUITE", "QV", "R", "RAN", "RATHER",
    "RD", "READILY", "REALLY", "RECENT", "RECENTLY", "REF", "REFS", "REGARDING",
    "REGARDLESS", "REGARDS", "RELATED", "RELATIVELY", "RESEARCH", "RESPECTIVELY",
    "RESULTED", "RESULTING", "RESULTS", "RIGHT", "RUN", "SAID", "SAW", "SAY", "SAYING",
    "SAYS", "SEC", "SECTION", "SEE", "SEEING", "SEEM", "SEEMED", "SEEMING", "SEEMS",
    "SEEN", "SELF", "SELVES", "SENT", "SEVEN", "SEVERAL", "SHALL", "SHED", "SHES", "SHOW",
    "SHOWED", "SHOWN", "SHOWNS", "SHOWS", "SIGNIFICANT", "SIGNIFICANTLY", "SIMILAR",
    "SIMILARLY", "SINCE", "SIX", "SLIGHTLY", "SOMEBODY", "SOMEHOW", "SOMEONE",
    "SOMETHAN", "SOMETHING", "SOMETIME", "SOMETIMES", "SOMEWHAT", "SOMEWHERE", "SOON",
    "SORRY", "SPECIFICALLY", "SPECIFIED", "SPECIFY", "SPECIFYING", "STILL", "STOP",
    "STRONGLY", "SUB", "SUBSTANTIALLY", "SUCCESSFULLY", "SUFFICIENTLY", "SUGGEST",
    "SUP", "SURE", "TAKE", "TAKEN", "TAKING", "TELL", "TENDS", "TH", "THANK", "THANKS",
    "THANX", "THATS", "THAT'VE", "THENCE", "THEREAFTER", "THEREBY", "THERED", "THEREFORE",
    "THEREIN", "THERE'LL", "THEREOF", "THERERE", "THERES", "THERETO", "THEREUPON",
    "THERE'VE", "THEYD", "THEYRE", "THINK", "THOU", "THOUGH", "THOUGHH", "THOUSAND",
    "THROUG", "THROUGHOUT", "THRU", "THUS", "TIL", "TIP", "TOGETHER", "TOOK", "TOWARD",
    "TOWARDS", "TRIED", "TRIES", "TRULY", "TRY", "TRYING", "TS", "TWICE", "TWO", "U",
    "UN", "UNFORTUNATELY", "UNLESS", "UNLIKE", "UNLIKELY", "UNTO", "UPON", "UPS", "US",
    "USE", "USED", "USEFUL", "USEFULLY", "USEFULNESS", "USES", "USING", "USUALLY", "V",
    "VALUE", "VARIOUS", "'VE", "VIA", "VIZ", "VOL", "VOLS", "VS", "W", "WANT", "WANTS",
    "WASNT", "WAY", "WED", "WELCOME", "WENT", "WERENT", "WHATEVER", "WHAT'LL", "WHATS",
    "WHENCE", "WHENEVER", "WHEREAFTER", "WHEREAS", "WHEREBY", "WHEREIN", "WHERES",
    "WHEREUPON", "WHEREVER", "WHETHER", "WHIM", "WHITHER", "WHOD", "WHOEVER", "WHOLE",
    "WHO'LL", "WHOMEVER", "WHOS", "WHOSE", "WIDELY", "WILLING", "WISH", "WITHIN",
    "WITHOUT", "WONT", "WORDS", "WORLD", "WOULDNT", "WWW", "X", "YES", "YET", "YOUD",
    "YOURE", "Z", "ZERO", "A'S", "AIN'T", "ALLOW", "ALLOWS", "APART", "APPEAR",
    "APPRECIATE", "APPROPRIATE", "ASSOCIATED", "BEST", "BETTER", "C'MON", "C'S",
    "CANT", "CHANGES", "CLEARLY", "CONCERNING", "CONSEQUENTLY", "CONSIDER", "CONSIDERING",
    "CORRESPONDING", "COURSE", "CURRENTLY", "DEFINITELY", "DESCRIBED", "DESPITE",
    "ENTIRELY", "EXACTLY", "EXAMPLE", "GOING", "GREETINGS", "HELLO", "HELP", "HOPEFULLY",
    "IGNORED", "INASMUCH", "INDICATE", "INDICATED", "INDICATES", "INNER", "INSOFAR",
    "IT'D", "KEEP", "KEEPS", "NOVEL", "PRESUMABLY", "REASONABLY", "SECOND", "SECONDLY",
    "SENSIBLE", "SERIOUS", "SERIOUSLY", "SURE", "T'S", "THIRD", "THOROUGH", "THOROUGHLY",
    "THREE", "WELL", "WONDER", "A", "ABOUT", "ABOVE", "ABOVE", "ACROSS", "AFTER",
    "AFTERWARDS", "AGAIN", "AGAINST", "ALL", "ALMOST", "ALONE", "ALONG", "ALREADY",
    "ALSO", "ALTHOUGH", "ALWAYS", "AM", "AMONG", "AMONGST", "AMOUNGST", "AMOUNT", "AN",
    "AND", "ANOTHER", "ANY", "ANYHOW", "ANYONE", "ANYTHING", "ANYWAY", "ANYWHERE", "ARE",
    "AROUND", "AS", "AT", "BACK", "BE", "BECAME", "BECAUSE", "BECOME", "BECOMES",
    "BECOMING", "BEEN", "BEFORE", "BEFOREHAND", "BEHIND", "BEING", "BELOW", "BESIDE",
    "BESIDES", "BETWEEN", "BEYOND", "BILL", "BOTH", "BOTTOM", "BUT", "BY", "CALL",
    "CAN", "CANNOT", "CANT", "CO", "CON", "COULD", "COULDNT", "CRY", "DE", "DESCRIBE",
    "DETAIL", "DO", "DONE", "DOWN", "DUE", "DURING", "EACH", "EG", "EIGHT", "EITHER",
    "ELEVEN", "ELSE", "ELSEWHERE", "EMPTY", "ENOUGH", "ETC", "EVEN", "EVER", "EVERY",
    "EVERYONE", "EVERYTHING", "EVERYWHERE", "EXCEPT", "FEW", "FIFTEEN", "FIFY", "FILL",
    "FIND", "FIRE", "FIRST", "FIVE", "FOR", "FORMER", "FORMERLY", "FORTY", "FOUND",
    "FOUR", "FROM", "FRONT", "FULL", "FURTHER", "GET", "GIVE", "GO", "HAD", "HAS",
    "HASNT", "HAVE", "HE", "HENCE", "HER", "HERE", "HEREAFTER", "HEREBY", "HEREIN",
    "HEREUPON", "HERS", "HERSELF", "HIM", "HIMSELF", "HIS", "HOW", "HOWEVER", "HUNDRED",
    "IE", "IF", "IN", "INC", "INDEED", "INTEREST", "INTO", "IS", "IT", "ITS", "ITSELF",
    "KEEP", "LAST", "LATTER", "LATTERLY", "LEAST", "LESS", "LTD", "MADE", "MANY", "MAY",
    "ME", "MEANWHILE", "MIGHT", "MILL", "MINE", "MORE", "MOREOVER", "MOST", "MOSTLY",
    "MOVE", "MUCH", "MUST", "MY", "MYSELF", "NAME", "NAMELY", "NEITHER", "NEVER",
    "NEVERTHELESS", "NEXT", "NINE", "NO", "NOBODY", "NONE", "NOONE", "NOR", "NOT",
    "NOTHING", "NOW", "NOWHERE", "OF", "OFF", "OFTEN", "ON", "ONCE", "ONE", "ONLY",
    "ONTO", "OR", "OTHER", "OTHERS", "OTHERWISE", "OUR", "OURS", "OURSELVES", "OUT",
    "OVER", "OWN", "PART", "PER", "PERHAPS", "PLEASE", "PUT", "RATHER", "RE", "SAME",
    "SEE", "SEEM", "SEEMED", "SEEMING", "SEEMS", "SERIOUS", "SEVERAL", "SHE", "SHOULD",
    "SHOW", "SIDE", "SINCE", "SINCERE", "SIX", "SIXTY", "SO", "SOME", "SOMEHOW",
    "SOMEONE", "SOMETHING", "SOMETIME", "SOMETIMES", "SOMEWHERE", "STILL", "SUCH",
    "SYSTEM", "TAKE", "TEN", "THAN", "THAT", "THE", "THEIR", "THEM", "THEMSELVES",
    "THEN", "THENCE", "THERE", "THEREAFTER", "THEREBY", "THEREFORE", "THEREIN",
    "THEREUPON", "THESE", "THEY", "THICKV", "THIN", "THIRD", "THIS", "THOSE",
    "THOUGH", "THREE", "THROUGH", "THROUGHOUT", "THRU", "THUS", "TO", "TOGETHER",
    "TOO", "TOP", "TOWARD", "TOWARDS", "TWELVE", "TWENTY", "TWO", "UN", "UNDER",
    "UNTIL", "UP", "UPON", "US", "VERY", "VIA", "WAS", "WE", "WELL", "WERE", "WHAT",
    "WHATEVER", "WHEN", "WHENCE", "WHENEVER", "WHERE", "WHEREAFTER", "WHEREAS", "WHEREBY",
    "WHEREIN", "WHEREUPON", "WHEREVER", "WHETHER", "WHICH", "WHILE", "WHITHER", "WHO",
    "WHOEVER", "WHOLE", "WHOM", "WHOSE", "WHY", "WILL", "WITH", "WITHIN", "WITHOUT",
    "WOULD", "YET", "YOU", "YOUR", "YOURS", "YOURSELF", "YOURSELVES", "THE", "A",
    "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q",
    "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
    "Y", "Z", "CO", "OP", "RESEARCH-ARTICL", "PAGECOUNT", "CIT", "IBID", "LES", "LE",
    "AU", "QUE",     "EST", "PAS", "VOL", "EL", "LOS", "PP", "U201D", "WELL-B", "HTTP",
    "VOLUMTYPE",     "PAR", "0O", "0S", "3A", "3B", "3D", "6B", "6O", "A1", "A2", "A3",
    "A4", "AB", "AC", "AD", "AE", "AF", "AG", "AJ", "AL", "AN", "AO", "AP", "AR",
    "AV", "AW", "AX", "AY", "AZ", "B1", "B2", "B3", "BA", "BC", "BD", "BE", "BI", "BJ",
    "BK", "BL", "BN", "BP", "BR", "BS", "BT", "BU", "BX", "C1", "C2", "C3", "CC", "CD",
    "CE", "CF", "CG", "CH", "CI", "CJ", "CL", "CM", "CN", "CP", "CQ", "CR", "CS", "CT",
    "CU", "CV", "CX", "CY", "CZ", "D2", "DA", "DC", "DD", "DE", "DF", "DI", "DJ", "DK",
    "DL", "DO", "DP", "DR", "DS", "DT", "DU", "DX", "DY", "E2", "E3", "EA", "EC", "ED",
    "EE", "EF", "EI", "EJ", "EL", "EM", "EN", "EO", "EP", "EQ", "ER", "ES", "ET", "EU",
    "EV", "EX", "EY", "F2", "FA", "FC", "FF", "FI", "FJ", "FL", "FN", "FO", "FR", "FS",
    "FT", "FU", "FY", "GA", "GE", "GI", "GJ", "GL", "GO", "GR", "GS", "GY", "H2", "H3",
    "HH", "HI", "HJ", "HO", "HR", "HS", "HU", "HY",  "I", "I2", "I3", "I4", "I6", "I7",
    "I8", "IA", "IB", "IC", "IE", "IG", "IH", "II", "IJ", "IL", "IN", "IO", "IP", "IQ",
    "IR", "IV", "IX", "IY", "IZ", "JJ", "JR", "JS", "JT", "JU", "KE", "KG", "KJ", "KM",
    "KO", "L2", "LA", "LB", "LC", "LF", "LJ", "LN", "LO", "LR", "LS", "LT", "M2", "ML",
    "MN", "MO", "MS", "MT", "MU", "N2", "NC", "ND", "NE", "NG", "NI", "NJ", "NL", "NN",
    "NR", "NS", "NT", "NY", "OA", "OB", "OC", "OD", "OF", "OG", "OI", "OJ", "OL", "OM",
    "ON", "OO", "OQ", "OR", "OS", "OT", "OU", "OW", "OX", "OZ", "P1", "P2", "P3", "PC",
    "PD", "PE", "PF", "PH", "PI", "PJ", "PK", "PL", "PM", "PN", "PO", "PQ", "PR", "PS",
    "PT", "PU", "PY", "QJ", "QU", "R2", "RA", "RC", "RD", "RF", "RH", "RI", "RJ", "RL",
    "RM", "RN", "RO", "RQ", "RR", "RS", "RT", "RU", "RV", "RY", "S2", "SA", "SC", "SD",
    "SE", "SF", "SI", "SJ", "SL", "SM", "SN", "SP", "SQ", "SR", "SS", "ST", "SY", "SZ",
    "T1", "T2", "T3", "TB", "TC", "TD", "TE", "TF", "TH", "TI", "TJ", "TL", "TM", "TN",
    "TP", "TQ", "TR", "TS", "TT", "TV", "TX", "UE", "UI", "UJ", "UK", "UM", "UN", "UO",
    "UR", "UT", "VA", "WA", "VD", "WI", "VJ", "VO", "WO", "VQ", "VT", "VU", "X1", "X2",
    "X3", "XF", "XI", "XJ", "XK", "XL", "XN", "XO", "XS", "XT", "XV", "XX", "Y2", "YJ",
    "YL", "YR", "YS", "YT", "ZI", "ZZ", 0
};

int main()
{
    Arena *context = malloc(sizeof(Arena));
    *context = (Arena){0};

    int count = 0;
    ltbs_cell *hashmap = Hash_Vt.new(context);
    ltbs_cell *actual_keys = List_Vt.nil();

    printf("\n----------------\n");
    printf("INSERTION START");
    printf("\n----------------\n");
    for ( int index = 0; STOP_WORD_LIST[index]; index++ )
    {
	ltbs_cell *stop_word = String_Vt.cs(STOP_WORD_LIST[index], context);
	actual_keys = List_Vt.cons(stop_word, actual_keys, context);
	
	Hash_Vt.upsert(
	    &hashmap,
	    stop_word,
	    List_Vt.from_int(1, context),
	    context
	);
	printf("%s, ", STOP_WORD_LIST[index]);
	count++;
    }
    printf("\n----------------\n");
    printf("INSERTION END");
    printf("\n----------------\n");

    ltbs_cell *keys = List_Vt.reverse(Hash_Vt.keys(&hashmap, context), context);

    {
	for ( ltbs_cell *tracker = keys;
	      List_Vt.head(tracker);
	      tracker = List_Vt.rest(tracker) )
	    printf("%s, ", List_Vt.head(tracker)->data.string.strdata);
    }

    /* { */
    /* 	for ( ltbs_cell *tracker = keys; */
    /* 	      List_Vt.head(tracker); */
    /* 	      tracker = List_Vt.rest(tracker) ) */
    /* 	    for ( ltbs_cell *inner = actual_keys; */
    /* 		  List_Vt.head(inner); */
    /* 		  inner = List_Vt.rest(inner) ) */
    /* 	    { */
    /* 		if ( String_Vt.compare(List_Vt.head(tracker), List_Vt.head(inner)) ) */
    /* 		{ */
    /* 		    printf("%s was added.\n", ); */
    /* 		} */
    /* 	    } */
		
    /* } */

    printf("\n");
    printf("\n----------------\n");
    printf("Actual count: %d\n", count);
    printf("Keys count: %d\n", List_Vt.count(keys));

    arena_free(context);
    free(context);
    return 0;
}
