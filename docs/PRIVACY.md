# Privacy statement

We are [A Bunch of Hacks](https://abunchofhacks.coop), the team behind
Epicinium. We are not interested in your personal data; we only store
what we need for you to play Epicinium and for us to improve it. We are
happy to explain what data we collect, how we store it and what we use
it for, as we generally do not believe in security through obscurity and
we sure as hell don't believe in a business model where we profit from
your data.

In order to be able to play Epicinium, you need to either launch the
game from the [Steam](https://steampowered.com) desktop client, or
register an account in-game.

When you launch Epicinium from Steam, the game transmits a temporary
session ticket to our server (we use a [TransIP](https://www.transip.nl)
VPS located in Amsterdam, The Netherlands). The server uses this ticket
to obtain and verify your **SteamID**, a number that [uniquely
identifies](https://support.steampowered.com/kb_article.php?ref=1558-qyax-1965)
your Steam account. The server also asks Steam for your **player name**
and uses it as the basis for your Epicinium **username**, which is how
you are represented to other players in-game and on public leaderboards.
Your SteamID and your Epicinium username are stored in a MySQL database.

When registering an account, we ask for your **email address**, a
**username** and a **password**. These are transmitted securely using a
[Let's Encrypt](https://letsencrypt.org/) TLS-encrypted connection. Your
password is hashed with a SHA-512 algorithm before transmission, so we
never get to see it. Once this data arrives at our server, we generate a
hash and salt for your already-hashed password using the
industry-standard PHP bcrypt password\_hash implementation. Then, we
store that and the rest of the data in our database.

By communicating to us over the internet (Epicinium is an online-only
game), you also implicitly transmit to us your **IP address**. Your IP
address is generally not stored in the database, except in particular
cases where we can use it for a specific security purpose. For example,
at an unsuccessful login attempt we store the IP address of the
attempter so we can later verify that it was you or block the
perpetrator. We also sometimes store a timestamp for debugging and
support purposes, such as the last time you successfully logged in. The
server is secured through best practices, including a firewall and an
intrusion prevention system. The database is regularly backed up to
prevent accidental data loss.

Additionally, we keep server logs that log the interaction between your
Epicinium game client and our game server. These logs are occasionally
used to help us with debugging and tracking down problems. For example,
if we find out people are having trouble logging in, if we think there
is a bug in the software, or if we receive a complaint from another
player about your in-game behavior, we sometimes take a look at these
logs to find out what's going on. These logs may contain timestamps,
your username, IP address and in-game chat messages.

We use your email address only for the following purposes:

-   **Account management**: This means we email you to facilitate your
    control over your Epicinium account. For example, when you first
    create your account we may send an email asking you to verify your
    email address, and when you indicate that you forgot your password,
    we can send you a password reset token. We also email you when we
    notice suspicious activity, like when someone unsuccessfully
    attempts to log in to your account.
-   **Updates** (only opt-in): If you indicate that you want to receive
    these on the Epicinium front page or during the registration
    process, we will send you occasional updates about the development
    of Epicinium. You can always unsubscribe by clicking the
    "unsubscribe" link in our emails.

If the game is launched from Steam, we share information about your
in-game activity (such as what map you are currently playing on or how
many games you've won in total) in order to facilitate Steam
Achievements and [Steam Rich
Presence](https://partner.steamgames.com/doc/features/enhancedrichpresence).
Versions of the game available elsewhere do not exchange any information
with Steam.

If the [Discord application](https://discord.com) is running while you
play Epicinium, the applications will share some information to enable
[Discord Rich Presence](https://discord.com/rich-presence)
functionality, specifically: your username on both services and metadata
on what you're doing in-game (waiting in a lobby, or playing versus AI
on a specific map, for instance). If you don't want this, you can
disable Discord Rich Presence in the Epicinium settings menu (or just
don't run Discord). This functionality will only activate after the
first login in Epicinium.

We handle your data with care. We would expect no less concerning our
own personal data. We will never sell your data to third parties. We
will not share your data with third parties, except strictly for the
express purposes outlined above (for example, we use Google Groups to
send updates).

We use your data on the basis of [legitimate
interest](https://gdpr-info.eu/recitals/no-47), as we need it to be able
to offer you a functional and secure online game. Our use of your email
address for sending updates (only when you opt in) is based on your
[consent](https://gdpr-info.eu/art-7-gdpr).

Information in our database dates back to March 2018, while our server
logs date back to November 2017. We currently do not have a fixed time
period for data retention because we need to retain it in order for you
to play Epicinium. Note however that you can always contact us to delete
your account, request an overview of your data, or remove all the
personal data we have of you (as well as any other [GDPR
right](https://gdpr-info.eu/chapter-3), whether you're an EU citizen or
not).

If you have any questions, requests or complaints regarding your
personal data or how we handle it, please don't hesitate to email us at
<support@epicinium.nl>.

— Daan and Sander ([A Bunch of Hacks](https://abunchofhacks.coop))
