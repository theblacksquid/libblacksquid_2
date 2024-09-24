

#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"
#define ARENA_IMPLEMENTATION
#define LIBBLACKSQUID_IMPLEMENTATION
#define LTBS_LIBXML2_IMPLEMENTATION

#include "libxml/xmlmemory.h"
#include <stddef.h>
#include <stdio.h>
#include "../libblacksquid.h"
#include "../ltbs_lxml2.h"
#include "libxml/tree.h"

Arena context = {0};

void null_free(void *mem) {}
void *malloc_with_arena(size_t size)
{
    return arena_alloc(&context, size);
}

void *realloc_with_arena(void *mem, size_t size)
{
    void *result = arena_alloc(&context, size);
    char *dest = (char *) result;
    char *source = (char *) mem;
    
    for ( int index = 0; index < size; index++ )
    {
        dest[index] = source[index];
    }

    return result;
}

char *strdup_with_arena(char *data)
{
    char *result;
    int length = 0;
    
    {
	for ( int index = 0; data[index]; index++ ) { length++; }	
    }

    result = arena_alloc(&context, length);
    *result = (char){0};

    {
	for ( int index = 0; index < length; index++ ) result[index] = data[index];
    }

    return result;
}

void print_node(xmlNodePtr node, void *___)
{
    char *content = Libxml2_vt.node_get_text(node);
    printf("content: %s\n", content);
}

void add_node_href_to_list(xmlNodePtr node, ltbs_cell **list)
{
    char *content = Libxml2_vt.node_get_prop(node, "href");

    if ( content )
    {
	ltbs_string *string = String_Vt.cs(content, &context);
	*list = List_Vt.cons(string, *list, &context);	
    }
}

int main()
{
    xmlMemSetup(null_free, malloc_with_arena, realloc_with_arena, strdup_with_arena);
    
    {
	char *xml1 =
	    "<article data-public='true'"
	    "         data-type='essay' "
	    "         data-filename='rss.htm'>"
	    "    <section aria-label='metadata'>"
	    "   	<h2 itemprop='title'>RSS, and why its awesome</h2>"
	    "   	<time date='2021-06-23'>2021-06-23</time>"
	    "    </section>"
	    "    <section aria-label='content'>"
	    "	<section id='infohazard-feeds'>"
	    "	    <h3>"
	    "		<a href='#infohazard-feeds'> # </a>"
	    "		The algorithmic feed as <a href='infohazards.htm'>"
	    "		    infohazard </a>"
	    "	    </h3>"
	    "	    <p>"
	    "		Twitter, <a href='the_facebook_papers.htm'>Facebook</a> and"
	    "		others have all taken over the majority of human interaction"
	    "		online, and profit off of selling our attention to the highest"
	    "		bidder. There is another, more insidious side (if that's even"
	    "		possible) to this, in that this can also be used to drown out"
	    "		narratives that aren't conducive to the status quo, as well as"
	    "		push agendas that do. We must always be on guard and have the"
	    "		<a href='sift_method.htm'>SIFT Method</a> ready when seeing"
	    "		extraordinary claims or emotionally-heated topics trending"
	    "		online."
	    "	    </p>"
	    "	    <p>"
	    "		Although one does wonder if there is an alternative to all of"
	    "		this. And, like most things related to software, an open standard"
	    "		has already been defined for the usecase of just wanting to read"
	    "		the goddamn news or blogs of friends and interesting people."
	    "	    </p>"
	    "	    <p>Enter <a href='https://en.wikipedia.org/wiki/RSS'>RSS</a>.</p>"
	    "	</section>"
	    "	<titled-section title='Freedom of Choice'"
	    "			section-id='freedom'>"
	    "	    <p>"
	    "		RSS or 'Really Simple Syndication', among other things, is a way for"
	    "		websites, normally news sites and blogs, to send content out as a"
	    "		feed. And yes, 'feed' in this context is the same as the Twitter and"
	    "		Facebook kinds. But the feed is ultimately generated and curated by"
	    "		the algorithms that the megacorps control."
	    "	    </p>"
	    "	    <p>"
	    "		RSS gets rid of all that crap by simply taking a list"
	    "		of feeds, usually just links to XML files served over"
	    "		the internet, and then just displays the posts to you"
	    "		in chronological order. The only time <a"
	    "		href='ai_weirdness.htm'> AI </a> was ever needed in"
	    "		creating a feed like this is for manipulating and"
	    "		selling peoples' attention."
	    "	    </p>"
	    "	</titled-section>"
	    "	<titled-section title='How to use it as a reader/subscriber'"
	    "			section-id='as-reader'>"
	    "	    <section>"
	    "		<h4>Get yourself a feed reader</h4>"
	    ""
	    "		<p>For Mobile (F-Droid Links)</p>"
	    "		<ul>"
	    "		    <li>"
	    "			<a href='https://f-droid.org/en/packages/ru.yanus171.feedexfork/'>"
	    "			    Handy News Reader"
	    "			</a>"
	    "		    </li>"
	    "		    <li>"
	    "			<a href='https://f-droid.org/en/packages/com.nononsenseapps.feeder'>"
	    "			    Feeder"
	    "			</a>"
	    "		    </li>"
	    "		</ul>"
	    "		<p>"
	    "		    Both of them support offline reading and background synchronization,"
	    "		    which are the features that one should look for in an RSS reader. "
	    "		</p>"
	    ""
	    "		<p>For the UNIX (Linux, *BSD's, etc) command line</p>"
	    "		<ul>"
	    "		    <li><a href='https://newsboat.org/'> Newsboard </a></li>"
	    "		</ul>"
	    "		"
	    "		<p>For PC desktop (Windows, Linux, MacOS)</p>"
	    "		<ul>"
	    "		    <li>"
	    "			<a href='https://support.mozilla.org/en-US/kb/how-subscribe-news-feeds-and-blogs'>"
	    "			    Mozilla Thunderbird"
	    "			</a>"
	    "		    </li>"
	    "		</ul>"
	    ""
	    "		<p>"
	    "		    I personally use"
	    ""
	    "		    <a href='https://github.com/skeeto/elfeed'>"
	    "			Elfeed"
	    "		    </a>"
	    ""
	    "		    and"
	    ""
	    "		    <a href='https://github.com/remyhonig/elfeed-org'>"
	    "			Elfeed-Org"
	    "		    </a>"
	    ""
	    "		    to manage and browser RSS feeds from within Emacs"
	    "		    as I spend most of my personal computing time"
	    "		    within it."
	    "		</p>"
	    "	    </section>"
	    "	    <section>"
	    "		<h4>Find yourself some feeds</h4>"
	    ""
	    "		<p>"
	    "		    After you have your RSS feed reader set up, it's"
	    "		    time to fill the feed with content. Go to your"
	    "		    favorite blogs and look for this symbol:"
	    "		</p>"
	    ""
	    "		<img alt='' src='/media/rss-icon.png'>rss-icon</img>"
	    ""
	    "		<p>"
	    "		    Or they might say something like 'Subscribe via RSS' like this blog"
	    "		    does. If you click on either, your RSS reader should pick it up or you"
	    "		    get prompted to select your RSS reader to open said link. "
	    "		</p>"
	    "	    </section>"
	    "	    <section>"
	    "		<h4>Getting RSS feeds from Youtube, Reddit, etc</h4>"
	    ""
	    "		<p>"
	    "		    Being a link aggregator, getting RSS feeds from"
	    "		    Reddit is fairly simple, just add"
	    "		    <code>.rss</code> to the end of the subreddit or"
	    "		    even username link, like so:"
	    "		</p>"
	    ""
	    "		<p>"
	    "		    <code>https://www.reddit.com/r/MechanicalKeyboards/.rss</code>"
	    "		</p>"
	    ""
	    "		<p>"
	    "		    And believe it or not, you can also get an RSS feed from youtube"
	    "		    channels, but it takes a bit more doing:"
	    "		</p>"
	    ""
	    "		<ol>"
	    "		    <li>"
	    "			<p>"
	    "			    Go and get the Channel ID of the channel"
	    "			    you want to follow"
	    "			</p>"
	    "			<p>"
	    "			    When you open a youtube channel page,"
	    "			    there should be what looks like gibberish"
	    "			    at the end of the URL, after the `/`"
	    "			    character:"
	    "			</p>"
	    "			<p>"
	    "			    <code>"
	    "https://www.youtube.com/channel/UCZFipeZtQM5CKUjx6grh54g"
	    "=> channel id is UCZFipeZtQM5CKUjx6grh54g"
	    "			    </code>"
	    "			</p>"
	    "		    </li>"
	    "		    <li>"
	    "			<p>"
	    "			    Append the channel ID to the link below"
	    "			</p>"
	    "			<p>"
	    "			    <code>https://www.youtube.com/feeds/videos.xml?channel_id=</code>"
	    "			</p>"
	    "			<p>"
	    "			    You need to add the channel ID right after"
	    "			    the <code>=</code> character, like so:"
	    "			</p>"
	    "			<p>"
	    "			    <code>https://www.youtube.com/feeds/videos.xml?channel_id=UCZFipeZtQM5CKUjx6grh54g</code>"
	    "			</p>"
	    "		    </li>"
	    "		    <li>"
	    "			<p>Add it to your chosen feed reader and enjoy!</p>"
	    "			<p>"
	    "			    Plus points if you can configure your feed reader to play youtube"
	    "			    videos in something like"
	    ""
	    "			    <a href='https://github.com/Feodor2/invidious'>"
	    "				Invidious"
	    "			    </a>"
	    ""
	    "			    or"
	    ""
	    "			    <a href='https://youtube-dl.org/'> youtube-dl </a>"
	    "			</p>"
	    "		    </li>"
	    "		</ol>"
	    ""
	    "		<p>"
	    "		    But then you might ask, if I hate megacorporations"
	    "		    so much, why am I teaching folks how to get"
	    "		    content from them? Mostly because we have to face"
	    "		    the facts that these corporations and their"
	    "		    government backers have"
	    ""
	    "		    <a href='class_struggle.htm'>"
	    "			so much control over our lives"
	    "		    </a>,"
	    ""
	    "		    both on- and offline. If there is a way we can"
	    "		    lessen their influence over us, we should take"
	    "		    it. Using RSS feeds as opposed to the"
	    "		    algorithmically-made ones fed to us by the"
	    "		    megacorps is one key step to doing that."
	    "		</p>"
	    "	    </section>"
	    "	    <section>"
	    "		<h4>Here's some RSS feeds I follow</h4>"
	    ""
	    "		<p>Some tech blogs I follow.</p>"
	    "		<code>"
	    "https://protesilaos.com/master.xml"
	    "https://drewdevault.com/blog/index.xml"
	    "https://100r.co/links/rss.xml"
	    "https://computer.rip/rss.xml"
	    "		</code>"
	    ""
	    "		<p>Misc. blogs that I follow</p>"
	    "		<code>"
	    "https://tendigits.space/feed.xml"
	    "https://grimgrains.com/links/rss.xml"
	    "		</code>"
	    ""
	    "		<p>Open Hardware and community platforms.</p>"
	    "		<code>"
	    "https://blog.freedombone.net/rss.xml"
	    "https://www.opensourceecology.org/feed/rss/"
	    "https://www.reddit.com/r/PiKeeb.rss"
	    "		</code>"
	    ""
	    "		<p>Advocacy and analysis</p>"
	    "		<code>"
	    "https://bandilangitim.noblogs.org/?feed=rss"
	    "http://libcom.org/rss.xml"
	    "		</code>"
	    ""
	    "		<p>"
	    "		    Some news sites in my feed, although I tend to filter them out in the"
	    "		    past few months to avoid doomscrolling. COVID19 and the"
	    ""
	    "		    <a href='climate_crisis.htm'> Climate Crisis </a>"
	    ""
	    "		    and all that."
	    "		</p>"
	    "		<code>"
	    "http://feeds.reuters.com/reuters/topNews (dead)"
	    "http://rss.cnn.com/rss/edition_asia.rss"
	    "https://www.aljazeera.com/xml/rss/all.xml"
	    "		</code>"
	    ""
	    "		<p>"
	    "		    Subreddits I follow"
	    "		</p>"
	    "		<code>"
	    "https://www.reddit.com/r/dwarffortress/.rss"
	    "https://www.reddit.com/r/emacs/.rss"
	    "https://www.reddit.com/r/MechanicalKeyboards/.rss"
	    "https://www.reddit.com/r/Brutalism.rss"
	    "		</code>"
	    "	    </section>"
	    "	</titled-section>"
	    "    </section>"
	    "</article>";

	
	xmlDocPtr result = Libxml2_vt.cstring_to_xml(xml1);
	xmlXPathObjectPtr query_result = Libxml2_vt.xpath(result, "//a");

	if ( query_result != NULL )
	{
	    Libxml2_vt.node_each(query_result, print_node, 0);
	    ltbs_cell *list = List_Vt.nil();
	    Libxml2_vt.node_each(query_result, add_node_href_to_list, &list);

	    pair_iterate(list, head, tracker,
            {
		printf("href: %s\n", head->data.string.strdata);
	    
	    });
	}

	else
	{
	    printf("no results\n");
	}
    }

    {
	printf("\n\n");
	char *path = "./test_data/rss.htm";
	htmlDocPtr result = Libxml2_vt.from_file(path);

	xmlXPathObjectPtr article = Libxml2_vt.xpath(result, "/article");
	xmlXPathObjectPtr links = Libxml2_vt.xpath(result, "//a");

	ltbs_cell *url_list = List_Vt.nil();

	Libxml2_vt.node_each(links, add_node_href_to_list, &url_list);
	
	char *post_type = Libxml2_vt.node_get_prop(
	    article->nodesetval->nodeTab[0],
	    "data-type"
	);

	char *post_public = Libxml2_vt.node_get_prop(
	    article->nodesetval->nodeTab[0],
	    "data-public"
	);

	char *post_filename = Libxml2_vt.node_get_prop(
	    article->nodesetval->nodeTab[0],
	    "data-filename"
	);

	char *post_title = Libxml2_vt.node_get_text(
	    Libxml2_vt.xpath(result, "//*[@itemprop = 'title']")->nodesetval->nodeTab[0]
	);

	char *post_date = Libxml2_vt.node_get_prop(
	    Libxml2_vt.xpath(result, "//time[@date]")->nodesetval->nodeTab[0],
	    "date"
	);

	printf(
	    "Title   : %s\n"
	    "Public  : %s\n"
	    "Filename: %s\n"
	    "Date    : %s\n"
	    "Type    : %s\n"
	    "Links   : \n",
	    post_title,
	    post_public,
	    post_filename,
	    post_date,
	    post_type
        );

	pair_iterate(url_list, head, tracker,
	{
	    printf("\t - %s\n", head->data.string.strdata);
	});
    }

    {
	printf("\n\n");

	char base_xml[] = "<article><p id='test1'></p><p id='test2'></p></article>";
	htmlDocPtr doc = Libxml2_vt.cstring_to_xml(base_xml);
	printf("%s\n\n\n", base_xml);

	xmlNodePtr article_node = Libxml2_vt.xpath(doc, "//article")->nodesetval->nodeTab[0];
	xmlNodePtr p1 = Libxml2_vt.xpath(doc, "//*[@id='test1']")->nodesetval->nodeTab[0];
	xmlNodePtr p2 = Libxml2_vt.xpath(doc, "//*[@id='test2']")->nodesetval->nodeTab[0];

	Libxml2_vt.node_set_prop(article_node, "data-testid", "testvalue");
	Libxml2_vt.node_set_text(p1, "This is some text");
	Libxml2_vt.node_set_prop(p2, "class", "card");
	Libxml2_vt.node_set_text(p2, "Another paragraph");

	char *output = Libxml2_vt.node_to_string(article_node);
	printf("%s\n", output);
    }

    arena_free(&context);
}
