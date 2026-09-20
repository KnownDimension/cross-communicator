#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <concord/discord.h>

u64snowflake Appid;
int i = 0;
int j = 0;
int counter;

struct context {
    char original_messageID[32];
    char original_guildID[32];
    char original_channelID[32];

    char destination_guildID[32];
    char destination_channelID[32];
};


struct ChannelGuildPair {
    char * ChannelID;
    char * GuildID;
};

struct ChannelGuildPair * pairs = NULL;


sqlite3 *db = NULL;
sqlite3_stmt *stmt = NULL;








void on_ready(struct discord *client, const struct discord_ready * event) {
    printf("\n on ready starts \n");
    int Check;
    char * statements = "SELECT * FROM channel_select;";
    Check = sqlite3_prepare_v2(db, statements, -1, &stmt, NULL);
    if (Check != SQLITE_OK) {
        fprintf(stderr, "database failed to initialise channels at the prepare stage: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(0);
    } 

    printf("\n database check success \n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        struct ChannelGuildPair * temp  = realloc(pairs, (counter + 1) * sizeof(struct ChannelGuildPair));

        if (temp == NULL) {
            printf("\n\n\n REALLOC HAS FAILED \n\n\n");
            break;
        } // checks if realloc failed

        pairs = temp; // allocates the pairs pointer to whatever temp was
        const char * GuildID = (const char *)sqlite3_column_text(stmt, 0);
        const char * ChannelID = (const char *)sqlite3_column_text(stmt, 1);

        printf("guild ID: %s \n channel ID: %s", GuildID, ChannelID);

        pairs[counter].GuildID = strdup(GuildID);
        pairs[counter].ChannelID = strdup(ChannelID); // must copy as pointer where the contents are stored may expire

        counter++; // used for dynamic allocation, when the while loop finishes, it is 1 above the full array number of values

    }
    sqlite3_finalize(stmt);
    




    Appid = event->application->id;

    struct discord_application_command_option channels[] = {
        {
            .type = DISCORD_APPLICATION_OPTION_CHANNEL,
            .name = "channel-select",
            .description = "select the channel you want the bot to be active in",
            .required = true,
            .channel_types = DISCORD_CHANNEL_GUILD_TEXT,
        },



    };

    struct discord_create_global_application_command channelConfig = {
        .name = "channel-configure",
        .description = "configures channel to use for cross communication",
    //    .default_permission = true,
        .default_member_permissions = 8,
        .options =
            &(struct discord_application_command_options){
                .size = sizeof(channels) / sizeof * channels, // divides the bite size of the entire array by one element in bytes of that array
                .array = channels,
            },
    };

    struct discord_create_global_application_command removeChannel = {
        .name = "channel-remove",
        .description = "remove this channel as the channel configured for cross communication",
    //    .default_permission = true,
        .default_member_permissions = 8,
    };

    discord_create_global_application_command(client, Appid, &channelConfig, NULL); // used to create a slash command for each guild in an array of guilds
    discord_create_global_application_command(client, Appid, &removeChannel, NULL); // used to create a slash command for each guild in an array of guilds



}








void on_interaction_create(struct discord * client, const struct discord_interaction * event) {

        /* We're only interested on slash commands */
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) return;
    /* Return in case user input is missing for some reason */
    if (!event->data && !strcmp(event->data->name, "channel-remove") == 0 || !event->data->options && !strcmp(event->data->name, "channel-remove") == 0) return;

    char * channelchosen = "tbd";

    if (strcmp(event->data->name, "channel-configure") == 0) {
        printf("\n\n adding channel guild combo \n\n");
        u64snowflake channelSnow = strtoull(event->data->options->array[0].value, NULL, 10);
        char guildSlash[32];
        char channelSlash[32];
        snprintf(guildSlash, sizeof(guildSlash), "%llu", event->guild_id);
        snprintf(channelSlash, sizeof(channelSlash), "%llu", channelSnow);

        const char * sqlStatement = "INSERT INTO channel_select VALUES(?, ?);";
        sqlite3_prepare_v2(db, sqlStatement, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, guildSlash, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, channelSlash, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        struct discord_create_message confirmation = { 
                .content = "channel configured for this discord server successfully, messages posted from other servers will now reflect to this channel"
            };
        discord_create_message(client, strtoull(channelSlash, NULL, 10), &confirmation, NULL);

    } else if (strcmp(event->data->name, "channel-remove") == 0) {
        printf("\n\n removing channel guild combo \n\n");
        const char * removeStatement = "DELETE FROM channel_select WHERE GuildID = ?;";
        char guildSlash[32];
        snprintf(guildSlash, sizeof(guildSlash), "%llu", event->guild_id);
        
        sqlite3_prepare_v2(db, removeStatement, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, guildSlash, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);



    }

}


void on_message_sent(struct discord * client, struct discord_response * returned, const struct discord_message * event) {
    printf("\n\n\n on message sent triggered");
    
    const struct context *origin = returned->data;


    printf("\n\n========== MESSAGE SENT CALLBACK ==========\n");

//    struct context *origin = returned->data;

    printf("Context pointer: %p\n", (void *)origin);

    printf(
        "Original Message ID: [%s]\n",
        origin->original_messageID
    );

    printf(
        "Context Destination Guild: [%s]\n",
        origin->destination_guildID
    );

    printf(
        "Context Destination Channel: [%s]\n",
        origin->destination_channelID
    );

    printf(
        "Discord response Channel ID: [%llu]\n",
        (unsigned long long)event->channel_id
    );

    printf(
        "Discord response Message ID: [%llu]\n",
        (unsigned long long)event->id
    );

    printf(
        "Discord response Guild ID: [%llu]\n",
        (unsigned long long)event->guild_id
    );

    printf("===========================================\n");


    char channel_id[32];
    char message_id[32];



    snprintf(channel_id, sizeof(channel_id), "%llu", event->channel_id);
    snprintf(message_id, sizeof(message_id), "%llu", event->id);


    const char * StoreMessage = "INSERT INTO cross_messages (OriginalMessageID, OriginalGuildID, OriginalChannelID, GuildID, ChannelID, MessageID) VALUES(?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, StoreMessage, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, origin->original_messageID, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, origin->original_guildID, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, origin->original_channelID, -1, SQLITE_TRANSIENT);

    sqlite3_bind_text(stmt, 4, origin->destination_guildID, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, channel_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, message_id, -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);




    free((void *)origin);

}






void on_message_create(struct discord *client, const struct discord_message *event) {
    printf("beginning guildID  before loop: %llu", event->guild_id);
    // bot message dupe prevention and channel check section
    if (event->author->bot) return;
    printf("\n pre everything check \n");
    printf("\n message recieved \n");
    int ChannelCheck = 0; // debug for nov
    int count2 = 0;
//    char * OID = NULL;
    char OID[32];

    //////// used for later
    struct MessageGuildLink {
        char * MessageID;
        char * GuildID;
        char * ChannelID;
    };
    struct MessageGuildLink MessageGuildPair[counter]; 
    //////// used for later


    for (i = 0; i < counter; i++) {
        if (strtoull(pairs[i].ChannelID, NULL, 10) == event->channel_id && strtoull(pairs[i].GuildID, NULL, 10) == event->guild_id) { 
            ChannelCheck = 1;
        }    
    }
    printf("\n%d channel check \n ", ChannelCheck);
    if (ChannelCheck == 0) return;
    printf("\n passed channel check \n ");




    const struct discord_guild *guild = discord_cache_get_guild(client, event->guild_id);
    // attachment handling section, so that attachment uploads work and are reflected to all the servers
    char messageContents[4096] = {0};
    snprintf(messageContents, sizeof(messageContents), "**from: %s   in: %s**\n\n%s", event->author->username, guild->name, event->content ? event->content : "");
    char attachURLS[4096] = {0};
    discord_unclaim(client, guild);



    if (event->attachments) { // if no images, the whole of this section isnt even touched
        printf("\n attachment detected \n ");
        for (i = 0; i < event->attachments->size; i++) {
            char temp[4096];
            snprintf(temp, sizeof(temp), "\n\n  %s", event->attachments->array[i].url);
            strncat(attachURLS, temp, sizeof(attachURLS) - strlen(attachURLS) - 1);
        }

        strncat(messageContents, attachURLS, sizeof(messageContents) - strlen(messageContents) - 1);

        if (strlen(messageContents) >= 2000) { // checks to see if adding the URL's will go beyond discords limits
            char ErrorReply[2048];
            snprintf(ErrorReply, sizeof(ErrorReply),"<@%llu>, due to your attached images or the first line message, and how the bot works, the message exceeded discords message limit, try seperating the text and the images if your writing super long messages", (unsigned long long)event->author->id);
            
            struct discord_create_message ReplyErr = {
                   .content = ErrorReply
            };
            discord_create_message(client, event->channel_id, &ReplyErr, NULL);
            return; // returns to prevent the execution of the rest of this event, ITS OVER

        }

    }



    


    // original message context section

   




    
    if (event->referenced_message) {
        printf("\n reference detected \n ");
        
        

        if (event->referenced_message->author->bot) {
            printf("\n bot reference detected \n ");
            char referenceStatement[512]; //must find the original message ID first
            snprintf(referenceStatement, sizeof(referenceStatement), "SELECT OriginalMessageID FROM cross_messages WHERE MessageID = '%llu';", (unsigned long long)event->referenced_message->id);
            sqlite3_prepare_v2(db, referenceStatement, -1, &stmt, NULL);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                strcpy(OID, sqlite3_column_text(stmt, 0));
                printf("OID original message: %s", OID );
                if (OID == NULL) {
                    printf("\nOID strdup failed, the referencing will probably break\n");
                }
            }
            sqlite3_finalize(stmt);
        } else {
            snprintf(OID, sizeof(OID), "%llu", (unsigned long long)event->referenced_message->id);
        }

    }


    char Guildchar[32];



    snprintf(Guildchar, sizeof(Guildchar), "%llu", (unsigned long long)event->guild_id);
    printf("\n start of message posting \n ");
    int messageCheck;
    int homereply = 0;

    // message posting section
    for (i = 0; i < counter; i++) {
        printf("\n current GuildID loop: %s \n Guildchar: %s", pairs[i].GuildID, Guildchar);
        if (strcmp(pairs[i].GuildID, Guildchar) == 0) continue;

        if (event->referenced_message) {
            char postStatement[1024] = "SELECT ChannelID, MessageID, GuildID FROM cross_messages WHERE OriginalMessageID = ? AND GuildID = ?;";
            printf("\n reference message posting \n ");
            char * Channeling = NULL;
            char * Guilding = NULL;
            char * Messaging = NULL;

            printf("\n===== REFERENCE LOOKUP =====\n");
            printf("OID: [%s]\n", OID);
            printf("Destination GuildID: [%s]\n", pairs[i].GuildID);
            printf("============================\n");

            messageCheck = sqlite3_prepare_v2(db, postStatement, -1, &stmt, NULL);
            if (messageCheck != SQLITE_OK) {
                printf("prepare failure: %s \n", sqlite3_errmsg(db));


            }
            sqlite3_bind_text(stmt, 1, OID, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, pairs[i].GuildID, -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {



                Messaging = strdup((char *)sqlite3_column_text(stmt, 1));
                Guilding = strdup((char *)sqlite3_column_text(stmt, 2));
                Channeling = strdup((char *)sqlite3_column_text(stmt, 0));

                printf("\n Messaging: %s \n Guilding: %s \n Channeling: %s", Messaging, Guilding, Channeling);

            }
            sqlite3_finalize(stmt);

            if (!Channeling || !Guilding || !Messaging) {
                if (homereply == 0) {
                    char postStatement2[1024] = "SELECT OriginalChannelID, OriginalMessageID, OriginalGuildID FROM cross_messages WHERE OriginalMessageID = ?;";
                    messageCheck = sqlite3_prepare_v2(db, postStatement2, -1, &stmt, NULL);
                    if (messageCheck != SQLITE_OK) {
                        printf("prepare failure: %s \n", sqlite3_errmsg(db));
                    }
                    sqlite3_bind_text(stmt, 1, OID, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, pairs[i].GuildID, -1, SQLITE_TRANSIENT);
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        Messaging = strdup((char *)sqlite3_column_text(stmt, 1));
                        Guilding = strdup((char *)sqlite3_column_text(stmt, 2));
                        Channeling = strdup((char *)sqlite3_column_text(stmt, 0));
                        printf("\n Messaging: %s \n Guilding: %s \n Channeling: %s", Messaging, Guilding, Channeling);
                        homereply = 1;
                    }

                } else {
                    printf("\n one of the channeling, guilding or messaging char's are NULL, message will not be mirrored\n");
                    free(Channeling);
                    free(Guilding);
                    free(Messaging);
                    continue;
                }
            }



            struct context * origin = malloc(sizeof(* origin));

            snprintf(origin->original_messageID, sizeof(origin->original_messageID),"%llu", event->id);
            snprintf(origin->original_guildID, sizeof(origin->original_guildID),"%llu", event->guild_id);
            snprintf(origin->original_channelID, sizeof(origin->original_channelID),"%llu", event->channel_id);



            snprintf(origin->destination_guildID, sizeof(origin->destination_guildID),"%s", pairs[i].GuildID);
            snprintf(origin->destination_channelID, sizeof(origin->destination_channelID),"%s", pairs[i].ChannelID);
            

            printf("\n allocated origin values \n ");
        

            struct discord_ret_message returned = {
                .done = on_message_sent,
                .data = origin,
            };

            printf("\n ret message struct known \n ");


            struct discord_create_message MessageType1 = { 
                   .content = messageContents,
                   .message_reference = &(struct discord_message_reference){
                        .message_id = strtoull(Messaging, NULL, 10),
                        .channel_id = strtoull(Channeling, NULL, 10),
                        .guild_id = strtoull(Guilding, NULL, 10),
                    },

            };


            discord_create_message(client, strtoull(Channeling, NULL, 10), &MessageType1, &returned);

            free(Channeling);
            free(Guilding);
            free(Messaging);

                
        } else if (!event->referenced_message) {
            printf("\n no reference message posting \n ");
            char * Channeling;
            Channeling = pairs[i].ChannelID;


            struct context * origin = malloc(sizeof(* origin));

            snprintf(origin->original_messageID, sizeof(origin->original_messageID),"%llu", event->id);
            snprintf(origin->original_guildID, sizeof(origin->original_guildID),"%llu", event->guild_id);
            snprintf(origin->original_channelID, sizeof(origin->original_channelID),"%llu", event->channel_id);



            snprintf(origin->destination_guildID, sizeof(origin->destination_guildID),"%s", pairs[i].GuildID);
            snprintf(origin->destination_channelID, sizeof(origin->destination_channelID),"%s", pairs[i].ChannelID);

            printf("\n allocated origin values \n ");


            struct discord_ret_message returned = {
                .done = on_message_sent,
                .data = origin,
            };

            printf("\n ret message struct known \n ");


            struct discord_create_message MessageType2 = { 
                .content = messageContents,
            };
            discord_create_message(client, strtoull(Channeling, NULL, 10), &MessageType2, &returned);
            printf("\n all stages complete \n ");
        } 
    }


}






int main () {

//    sqlite3 *db;
    char * dbErrorMessage;
    sqlite3_stmt *stmt;

    int Database = sqlite3_open("data.db", &db);

    if (Database != SQLITE_OK) {
        fprintf(stderr, "cant open database: %s/n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(0);
    } // opening the database and checking if its opened without errors, if error, print message and return


    if (access("config.json", F_OK) == -1) { // checks if config file even exists
        printf("config.json does not exist, pls make one");
        return -1;
    }

    const char * config_file;
    config_file = "config.json"; // confic file name

    struct discord * client = discord_config_init(config_file); //  initialises a client based on the config file

    ccord_global_init(); // initialises shared resources
    discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);
    discord_set_on_ready(client, &on_ready);
    discord_set_on_interaction_create(client, &on_interaction_create);
    discord_set_on_message_create(client, &on_message_create);
    discord_cache_enable(client, DISCORD_CACHE_GUILDS);


    discord_run(client); // run


    
    discord_cleanup(client); // cleanup when its all ended
    ccord_global_cleanup();

    for (i = 0; i < counter; i++) {
        free(pairs[i].GuildID);
        free(pairs[i].ChannelID);
    }
    free(pairs);

    sqlite3_close(db);




}
