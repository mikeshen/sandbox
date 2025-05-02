import pandas as pd
from flask import Flask, render_template, request
from google import genai
import os
import praw
import json
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

app = Flask(__name__)

# Configure the API key from environment variable
genai_api_key = os.environ.get("GENAI_API_KEY")
if not genai_api_key:
    raise ValueError("GENAI_API_KEY environment variable is required")
client = genai.Client(api_key=genai_api_key)

# Get Reddit credentials from environment variables
reddit_client_id = os.environ.get("REDDIT_CLIENT_ID")
reddit_client_secret = os.environ.get("REDDIT_CLIENT_SECRET")
reddit_user_agent = os.environ.get("REDDIT_USER_AGENT")

if not reddit_client_id or not reddit_client_secret:
    raise ValueError("REDDIT_CLIENT_ID and REDDIT_CLIENT_SECRET environment variables are required")

reddit = praw.Reddit(
    client_id=reddit_client_id,
    client_secret=reddit_client_secret,
    user_agent=reddit_user_agent
)

def fetch_posts(username, post_limit=10):
    user = reddit.redditor(username)
    return list(user.submissions.top(limit=post_limit, time_filter='all'))

def fetch_comments(username, comment_limit=10):
    user = reddit.redditor(username)
    return list(user.comments.top(limit=comment_limit, time_filter='all'))

def get_user_comments(username, top_limit=250, new_limit=250):
    """
    Fetches a user's top and new comments, enriches with parent info,
    and returns a pandas DataFrame.
    """
    user = reddit.redditor(username)
    comments_top = list(user.comments.top(limit=top_limit, time_filter='all'))
    comments_new = list(user.comments.new(limit=new_limit))

    # Deduplicate comments between comments_top and comments_new
    comments_top_ids = {comment.id for comment in comments_top}
    comments_new = [comment for comment in comments_new if comment.id not in comments_top_ids]

    def get_parent_info(comment):
        try:
            parent = comment.parent()
            if isinstance(parent, praw.models.Comment):
                return True, parent.body
        except Exception:
            pass
        return False, None

    comments = (
        [{'Body': c.body, 'Score': c.score, 'Created': c.created_utc,
          'Subreddit': str(c.subreddit), 'Permalink': c.permalink, 'Source': 'Top'}
         for c in comments_top]
        + [{'Body': c.body, 'Score': c.score, 'Created': c.created_utc,
            'Subreddit': str(c.subreddit), 'Permalink': c.permalink, 'Source': 'New'}
           for c in comments_new]
    )



    df = pd.DataFrame(comments)
    df['HasParent'], df['ParentBody'] = zip(*(get_parent_info(c) for c in comments))
    return df

def generate_analysis_prompt(df):
    """
    Generates a detailed textual analysis prompt based on a user's Reddit comments.

    The prompt is designed to infer the user's dominant interests, perspectives, and
    engagement style by analyzing the content, tone, and recurring themes in their comments.

    Parameters:
    - df (pd.DataFrame): A DataFrame containing the user's Reddit comments, enriched with
        metadata such as parent comment, score, subreddit, and permalink.

    Returns:
    - str: A formatted string combining the analysis prompt and the user's comments.
    """

    prompt = """Undertake a textual analysis of the following collection of Reddit comments authored by a single user. Your primary objective is to systematically infer the user's dominant interests and prevailing perspectives, strictly based on the observable content, tone, and recurring themes within these comments.

For each comment, meticulously consider the comment body, the specific subreddit, its received score, and the immediate context of the discussion.

Your analysis should move beyond a mere enumeration of mentioned topics. Instead, synthesise the gathered information to construct a structured overview that elucidates what this individual demonstrably engages with, thinks about, and potentially values, as evidenced by their online contributions.

Specifically, your overview should address the following:

Salient Topical Clusters: Identify and describe the most frequently recurring subjects or areas of discussion. Provide illustrative examples from the comments.
Dominant Stances: For each identified topical cluster, articulate the user's typical viewpoint. Is their engagement characterised by critique, advocacy, curiosity, scepticism, or another discernible stance? Support your assessment with textual evidence.
Linguistic Fingerprint: Analyse the user's tone and style of communication. Is their language predominantly technical, informal, confrontational, humorous, supportive, or cynical? What inferences can be drawn about their engagement style or potential online persona from this linguistic fingerprint?
Inferred Areas of Engagement: Based on the context of their comments and the subreddits they participate in, what potential hobbies, areas of professional involvement, intellectual curiosities, or lifestyle indicators can be reasonably inferred? Clearly state the basis for each inference.
Potential Areas of Value or Concern: Identify any explicitly stated or consistently implied values or concerns that underpin the user's contributions. Focus on what they repeatedly emphasize or argue for.
Identify Biases: Highlight any evident biases or predispositions in the user's comments. Are there particular ideologies, communities, or perspectives they seem to favour or oppose? Provide examples to substantiate your claims.
Internal Consistency and Evolution: Where the data permits, note any potential contradictions in the user's expressed views or any discernible shifts in their perspective across different comments or over time.
Roast the User: If you were to provide a humorous or light-hearted critique of the user's online persona based on their comments, what would it be? This should be done harshly and brutally honestly, but in a humorous way.
Present your findings in a logically structured report, beginning with the most prominent observations and progressing to more nuanced interpretations. Maintain a critical and discerning approach, acknowledging the inherent limitations of inferring user characteristics from textual data alone. Avoid definitive pronouncements about the user's intrinsic personality; instead, focus rigorously on the observable patterns of interest and perspective within the provided comments."""

    formatted_comments = "\n\n".join(
        (f"↳ Parent Comment:\nBody: {row['ParentBody']}\n" if row['HasParent'] else "")
        + f"OP Comment:\nBody: {row['Body']}\nScore: {row['Score']}\nCreated: {row['Created']}\nSubreddit: {row['Subreddit']}\nSource: {row['Source']}\nLink: https://www.reddit.com{row['Permalink']}\n"
        for _, row in df.iterrows()
    )

    analysis_input = prompt + "\n\n" + formatted_comments
    return analysis_input

@app.route('/')
def index():
    return '''
        <html>
        <head>
            <title>Reddit User Profiler</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    background-color: #f9f9f9;
                    margin: 0;
                    padding: 0;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                }
                .container {
                    background: #ffffff;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    text-align: center;
                    width: 300px;
                }
                h1 {
                    font-size: 1.5em;
                    margin-bottom: 20px;
                    color: #333;
                }
                label {
                    display: block;
                    margin-bottom: 8px;
                    font-weight: bold;
                    color: #555;
                }
                input[type="text"] {
                    width: 100%;
                    padding: 8px;
                    margin-bottom: 16px;
                    border: 1px solid #ccc;
                    border-radius: 4px;
                }
                button {
                    background-color: #007BFF;
                    color: white;
                    border: none;
                    padding: 10px 15px;
                    border-radius: 4px;
                    cursor: pointer;
                    font-size: 1em;
                }
                button:hover {
                    background-color: #0056b3;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>Reddit User Profiler</h1>
                <form method="post" action="/fetch">
                    <label for="username">Reddit Username:</label>
                    <input type="text" id="username" name="username" placeholder="Enter username, no u/ required" required>
                    <button type="submit">Analyze</button>
                </form>
            </div>
        </body>
        </html>
    '''


@app.route('/fetch', methods=['POST'])
def fetch():
    username = request.form.get('username')
    if not username:
        return "Username is required", 400

    try:
        df = get_user_comments(username)
        data = df.to_dict(orient='records')
        cols = df.columns.tolist()
        prompt = generate_analysis_prompt(df)
        response = client.models.generate_content(model="gemini-2.0-flash", contents=prompt)


        return render_template(
            'dataframe.html',
            username=username,
            data=data,
            columns=cols,
            analysis=response.text,
        )
    except Exception as e:
        return f"An error occurred: {str(e)}", 500

if __name__ == '__main__':
    # Run the Flask app
    app.run(debug=True)

